#include "battery.h"
#include "rtcMem/rtcMem.h"
#include "esp_pm.h"
#include "esp_wifi.h"

// Forward declarations for LED Display power methods
void disableLedDisplay();
void enableLedDisplay();
extern long getTimeUntilNextAlarm();

int batteryPercentage;
float batteryVoltage;

bool powerConnected = false;
bool charging = false;
bool wentToSleep = false;

// Task handle to allow external interrupts to wake manageBattery immediately
TaskHandle_t batteryTaskHandle = NULL;

void enableSleep();
void initSleep();
void enableAllSensors();
void disableAllSensors();
void manageBattery(void *parameter);
void controlCharger();
void wakeUpAndRestoreState();

void enableAllSensors()
{
  enableColorSensor();
  enableLightSensor();
  enablePressureSensor();
  enableTempSensor();
  setTouchNormalPower();
  rM.gpioExpander.setDefaultPinStates();
}

void disableAllSensors()
{
  disableColorSensor();
  disableLightSensor();
  disablePressureSensor();
  disableTempSensor();
  setTouchLowPower();
  rM.gpioExpander.enterLowPowerState();
}

void wakeUpAndRestoreState()
{
  enableAllSensors();
  vTaskResume(oledWakeupTaskHandle);
  vTaskResume(TimeTask);
  vTaskResume(dimmingTaskHandle);
  vTaskResume(alarmTaskHandle);
  if (!ringing)
  {
    vTaskResume(menuTaskHandle);
  }
  oledMana.enable();
  enableLedDisplay();
}

void createBatteryTask()
{
  xTaskCreate(
      manageBattery,     // Function to implement the task
      "Battery",         // Task name
      4096,              // Stack size (words)
      NULL,              // Task input parameter
      3,                 // Priority (0 is lowest)
      &batteryTaskHandle // Task handle assigned for event notifications
  );
}

bool checkPower()
{
  int chargingState = rM.gpioExpander.digitalRead(MCP_5V);
  Serial.print("Charging State: ");
  Serial.println(chargingState);
  if (chargingState == HIGH)
  {
    powerConnected = true;
    return true;
  }
  else
  {
    powerConnected = false;
    return false;
  }
}

bool batterySleepMode = false;
bool wokeUp = true;

void manageBattery(void *parameter)
{
  const unsigned long batCheInterval = 15000;
  const unsigned long batteryWaitTimeout = 3000;

  unsigned long lastRunBatChe = 0;
  unsigned long batterySettingsTime = 0;
  unsigned long wakeupTime = 0;
  unsigned long initialWakeupTime = 0; // Tracks when the active wake cycle started

  bool setPowerSettings = true;
  bool waitingForPower = false;
  bool waitForInput = false;

  powerConnected = checkPower();
  batteryVoltage = getBatteryVoltage();
  controlCharger();

  while (true)
  {
    // Block task for up to 100ms or until notified by a hardware interrupt/event.
    // This enables FreeRTOS Tickless Idle to automatically enter light sleep.
    ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(100));

    unsigned long now = millis();
    if (now - lastRunBatChe >= batCheInterval)
    {
      lastRunBatChe = now;
      batteryVoltage = getBatteryVoltage();
      if (powerConnected == true)
      {
        controlCharger();
      }
    }

    if (powerConnected)
    {
      if (!setPowerSettings)
      {
        Serial.println("Setting power settings (AC Mode)");
        wakeUpAndRestoreState();

        setPowerSettings = true;
        batterySleepMode = false;
        waitingForPower = false;
        esp_wifi_start();
        wokeUp = true;
      }

      if (!WiFi.isConnected() && !WifiTaskRunning)
      {
        esp_wifi_start();
        Serial.println("Launching WiFi task");
        createWifiTask();
      }
    }
    else
    {
      if (setPowerSettings)
      {
        Serial.println("Setting battery settings (Low-Power DFS Mode)");

        // Shut down Wi-Fi stack and RF domain fully to save max power
        turnOffWifi();
        esp_wifi_stop();
        esp_wifi_deinit();

        rM.gpioExpander.setPinState(MCP_CHARGER_CONTROL_PIN, false);
        batterySettingsTime = now;
        waitingForPower = true;
        setPowerSettings = false;
        maxBrightness = false;
        inputDetected = false;
      }

      if (waitingForPower == true)
      {
        if ((useAllButtons() != None || useAllTouch().touched == true || inputDetected == true) || ringing == true)
        {
          waitForInput = true;
          inputDetected = false;
          wakeUpAndRestoreState();
          setLedIntensity(2);
          showCurrentTime();
          batterySettingsTime = now;
        }

        if (waitingForPower && (now - batterySettingsTime >= batteryWaitTimeout))
        {
          batterySleepMode = true;
          waitingForPower = false;
          Serial.println("Battery mode active");
          syncESP32RTC();
          enableSleep();
          
          // Re-evaluate current time immediately after returning from enableSleep()
          now = millis();
          wakeupTime = now;
          initialWakeupTime = now;
        }
      }

      if (batterySleepMode)
      {
        esp_sleep_wakeup_cause_t wakeup_cause = esp_sleep_get_wakeup_cause();

        if (wakeup_cause == ESP_SLEEP_WAKEUP_TIMER)
        {
          if (!wokeUp)
          {
            Serial.println("Woke up from Timer, waiting for input...");
            wokeUp = true;
            waitForInput = false;
            wakeupTime = now;
            initialWakeupTime = now; // Initialize total awake session timer
          }

          // If an input or alarm occurs during timer awake, wake up sensors & LED display
          if ((useAllButtons() != None || useAllTouch().touched == true || inputDetected == true) || ringing == true)
          {
            inputDetected = false;
            if (waitForInput == false)
            {
              Serial.println("Input or Alarm detected during timer wake");
              wakeUpAndRestoreState();
              waitForInput = true;
              setLedIntensity(2);
              showCurrentTime();
            }
            Serial.println("Resetting sleep timer");
            delay(10);
            wakeupTime = now;
          }

          // Enforce maximum 5-minute awake cap
          if (now - initialWakeupTime >= MAX_AWAKE_HARD_LIMIT)
          {
            Serial.println("Hard 5-minute awake limit reached, forcing sleep");
            waitForInput = false;
            enableSleep();
          }
          else if (waitForInput == false)
          {
            if (now - wakeupTime >= TIMER_WAKUP_TIME)
            {
              Serial.println("No input with timer, going back to sleep");
              waitForInput = false;
              enableSleep();
            }
          }
          else
          {
            if (now - wakeupTime >= GPIO_WAKUP_TIME)
            {
              Serial.println("No input with active session, going back to sleep");
              waitForInput = false;
              enableSleep();
            }
          }
        }
        else if (wakeup_cause == ESP_SLEEP_WAKEUP_EXT1)
        {
          if (!wokeUp)
          {
            Serial.println("Woke up from Interrupt (EXT1)");
            wokeUp = true;
            wakeupTime = now;
            initialWakeupTime = now;
            inputDetected = true;
            enableAllSensors(); // Restore sensors immediately on hardware interrupt
          }

          if ((useAllButtons() != None || useAllTouch().touched == true || inputDetected == true) || ringing == true)
          {
            inputDetected = false;
            if (waitForInput == false)
            {
              Serial.println("Input or Alarm detected during interrupt wake");
              wakeUpAndRestoreState();
              waitForInput = true;
              setLedIntensity(2);
              showCurrentTime();
            }
            Serial.println("Resetting sleep timer");
            delay(10);
            wakeupTime = now;
          }

          // Enforce maximum 5-minute awake cap
          if (now - initialWakeupTime >= MAX_AWAKE_HARD_LIMIT)
          {
            Serial.println("Hard 5-minute awake limit reached, forcing sleep");
            waitForInput = false;
            enableSleep();
          }
          else if (now - wakeupTime >= GPIO_WAKUP_TIME)
          {
            Serial.println("No input after interrupt, going back to sleep");
            waitForInput = false;
            enableSleep();
          }
        }
      }
    }
  }
}

void controlCharger()
{
  bool newChargingState = charging;

  if (charging && batteryVoltage >= BATT_TARGET_VOLTAGE)
  {
    newChargingState = false;
    Serial.println("Charging stopped (target voltage reached).");
  }
  else if (!charging && batteryVoltage <= (BATT_TARGET_VOLTAGE - BATT_HYSTERESIS))
  {
    newChargingState = true;
    Serial.println("Charging started (voltage dropped).");
  }

  if (newChargingState != charging)
  {
    charging = newChargingState;
    if (charging)
    {
      rM.gpioExpander.setPinPullUp(MCP_CHARGER_CONTROL_PIN, true);
    }
    else
    {
      rM.gpioExpander.setPinPullUp(MCP_CHARGER_CONTROL_PIN, false);
    }
    rM.gpioExpander.setPinState(MCP_CHARGER_CONTROL_PIN, charging);
  }

  Serial.print("Battery Voltage: ");
  Serial.print(batteryVoltage);
  Serial.print(" V - Charging: ");
  Serial.println(charging ? "ON" : "OFF");
}

uint64_t pinToMask(uint8_t pin)
{
  return ((uint64_t)(((uint64_t)1) << pin));
}

void initSleep()
{
  // Always configure hardware interrupts (Touch, MCP Expander)
  esp_sleep_enable_ext1_wakeup(
      (1ULL << TOUCH_INTERRUPT) | (1ULL << MCP_INTERRUPT_PIN),
      ESP_EXT1_WAKEUP_ANY_LOW);

  // Get remaining time until the next alarm in seconds
  long secondsToNextAlarm = getTimeUntilNextAlarm();

  if (secondsToNextAlarm > 0)
  {
    // Convert seconds to microseconds for ESP32 timer wakeup
    uint64_t sleepMicros = (uint64_t)secondsToNextAlarm * 1000000ULL;
    esp_sleep_enable_timer_wakeup(sleepMicros);
    Serial.printf("Timer wakeup set for next alarm in %ld seconds.\n", secondsToNextAlarm);
  }
  else
  {
    // No upcoming active alarm found: Disable timer wakeup entirely
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_TIMER);
    Serial.println("No active alarms found. Timer wakeup disabled (Device will only wake on GPIO/Interrupt).");
  }
}

void enableSleep()
{
  // Pre-Sleep Input Guard Check: Abort sleep if input or alarm was triggered before disabling peripherals
  if (useAllButtons() != None || useAllTouch().touched == true || inputDetected == true || ringing == true)
  {
    Serial.println("Sleep aborted: Pending input detected before peripheral shutdown.");
    wakeUpAndRestoreState();
    return;
  }

  // Turn off display peripherals
  oledMana.disable();
  disableLedDisplay();

  // Put all external sensors into low-power / sleep modes
  disableAllSensors();
  initSleep();
  wokeUp = false;

  // Suspend active tasks right before entering sleep mode
  vTaskSuspend(oledWakeupTaskHandle);
  vTaskSuspend(TimeTask);
  vTaskSuspend(dimmingTaskHandle);
  vTaskSuspend(alarmTaskHandle);
  vTaskSuspend(menuTaskHandle);

  // Final sanity check right before light sleep call
  if (useAllButtons() != None || useAllTouch().touched == true || inputDetected == true || ringing == true)
  {
    Serial.println("Sleep aborted: Pending input detected right before sleep start.");
    wakeUpAndRestoreState();
    return;
  }

  esp_err_t sleep_result = esp_light_sleep_start();

  // Handle sleep failure or immediate return
  if (sleep_result != ESP_OK)
  {
    Serial.printf("Sleep attempt failed or rejected (Error: %d). Restoring tasks.\n", sleep_result);
    wakeUpAndRestoreState();
    return;
  }

  Serial.println("Light sleep entered and woke up successfully.");

  // Always restore peripherals & tasks when leaving sleep mode
  wakeUpAndRestoreState();

  syncTimeLibWithRTC();
  checkAlarms();
  delay(200);
}

double readVoltage(byte pin)
{
  int reading = analogRead(pin);

  constexpr double c4 = -1.6000000000000e-14;
  constexpr double c3 =  1.1817100000000e-10;
  constexpr double c2 = -3.0121169100000e-07;
  constexpr double c1 =  1.1090192717940e-03;
  constexpr double c0 =  3.4143524634089e-02;

  double voltage = (((c4 * reading + c3) * reading + c2) * reading + c1) * reading + c0;

  return voltage * 1000.0; 
}

float getBatteryVoltage()
{
  double milliVolts = readVoltage(VOLTAGE_DIVIDER_PIN);

  Serial.print("milliVolts = ");
  Serial.println(milliVolts);
  milliVolts += ADC_OFFSET;
  float batteryVoltage = milliVolts / ADC_VOLTAGE_DIVIDER;

  Serial.print("batteryVoltage = ");
  Serial.println(batteryVoltage, 6);

  return batteryVoltage;
}

int getBatteryPercentage()
{
  int percentage = ((batteryVoltage - MIN_VOLTAGE) / (MAX_VOLTAGE - MIN_VOLTAGE)) * 100.00;
  percentage = min(percentage, 100);
  if (percentage < 0)
  {
    percentage = 0;
    return percentage;
  }
  else
  {
    return percentage;
  }
}