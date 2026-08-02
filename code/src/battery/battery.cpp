#include "battery.h"
#include "rtcMem/rtcMem.h"

int batteryPercentage;
float batteryVoltage;

bool powerConnected = false;
bool charging = false;

bool wentToSleep = false;

void enableSleep();
void initSleep();

void manageBattery(void *parameter);

void controlCharger();

void createBatteryTask()
{
  xTaskCreate(
      manageBattery, // Function to implement the task
      "Battery",     // Task name
      4096,          // Stack size (words)
      NULL,          // Task input parameter
      3,             // Priority (0 is lowest)
      NULL           // Task handle
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

  bool setPowerSettings = true;
  bool waitingForPower = false;
  bool waitForInput = false;

  powerConnected = checkPower();
  batteryVoltage = getBatteryVoltage();
  controlCharger();

  while (true)
  {
    delay(pdMS_TO_TICKS(100));
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
        Serial.println("Setting power settings");
        // touchSetCycles(0x500, 0x500);
        // lightMeter.setActiveMode();
        vTaskResume(oledWakeupTaskHandle);
        vTaskResume(TimeTask);
        vTaskResume(alarmTaskHandle);
        vTaskResume(menuTaskHandle);
        // setTouchInterrupt(TOUCH_1_Seg_PIN, TOUCH_1_Seg_THRESHOLD);
        // setTouchInterrupt(TOUCH_2_Seg_PIN, TOUCH_2_Seg_THRESHOLD);
        // setTouchInterrupt(TOUCH_3_Seg_PIN, TOUCH_3_Seg_THRESHOLD);
        // setTouchInterrupt(TOUCH_4_Seg_PIN, TOUCH_4_Seg_THRESHOLD);
        // setTouchInterrupt(TOUCH_5_Seg_PIN, TOUCH_5_Seg_THRESHOLD);
        esp_pm_config_t pm_config = {
            .max_freq_mhz = 80,
            .min_freq_mhz = 10,
            .light_sleep_enable = true,
        };
        esp_pm_configure(&pm_config);
        setPowerSettings = true;
        batterySleepMode = false;
        waitingForPower = false;
        esp_wifi_start();
        wokeUp = true;
      }

      if (!WiFi.isConnected() && !WifiTaskRunning)
      {
        esp_wifi_start();
        Serial.println("launching WiFi task");
        createWifiTask();
      }
    }
    else
    {

      if (setPowerSettings)
      {
        Serial.println("Setting battery settings");
        turnOffWifi();
        // touchSetCycles(0x5000, 0x5000);
        vTaskSuspend(oledWakeupTaskHandle);
        vTaskSuspend(TimeTask);
        vTaskSuspend(dimmingTaskHandle);
        vTaskSuspend(alarmTaskHandle);
        vTaskSuspend(menuTaskHandle);
        // setTouchInterrupt(TOUCH_1_Seg_PIN, TOUCH_1_Seg_THRESHOLD_BAT);
        // setTouchInterrupt(TOUCH_2_Seg_PIN, TOUCH_2_Seg_THRESHOLD_BAT);
        // setTouchInterrupt(TOUCH_3_Seg_PIN, TOUCH_3_Seg_THRESHOLD_BAT);
        // setTouchInterrupt(TOUCH_4_Seg_PIN, TOUCH_4_Seg_THRESHOLD_BAT);
        // setTouchInterrupt(TOUCH_5_Seg_PIN, TOUCH_5_Seg_THRESHOLD_BAT);
        rM.gpioExpander.setPinState(MCP_CHARGER_CONTROL_PIN, false);
        // lightMeter.setStandbyMode();
        batterySettingsTime = now;
        waitingForPower = true;
        setPowerSettings = false;
        maxBrightness = false;
        inputDetected = false;
        esp_wifi_stop();
        esp_pm_config_t pm_config = {
            .max_freq_mhz = 40,
            .min_freq_mhz = 10,
            .light_sleep_enable = true,
        };
        esp_pm_configure(&pm_config);
      }

      if (waitingForPower == true)
      {
        if ((useAllButtons() != None || useAllTouch().touched == true || inputDetected == true) || ringing == true)
        {
          waitForInput = true;
          vTaskResume(menuTaskHandle);
          inputDetected = false;
          oledMana.enable();
          setLedIntensity(2);
          showCurrentTime();
          // if (useAllTouch() != No_Seg)
          // {
          //   useTouch();
          // }

          // if (useAllButtons() != None)
          // {
          //   useButton();
          // }
          batterySettingsTime = now;
        }

        if (waitingForPower && (now - batterySettingsTime >= batteryWaitTimeout))
        {
          batterySleepMode = true;
          waitingForPower = false;
          Serial.println("Battery mode");
          syncESP32RTC();
          enableSleep();
        }
      }

      if (batterySleepMode)
      {
        if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_TIMER)
        {
          if (!wokeUp)
          {
            Serial.println("Woke up, waiting for input...");
            wokeUp = true;
            waitForInput = false;
            wakeupTime = now;
          }

          if ((useAllButtons() != None || useAllTouch().touched == true || inputDetected == true) || ringing == true)
          {
            inputDetected = false;
            if (waitForInput == false)
            {
              Serial.println("Input or Alarm detected waiting more in timer");
              vTaskResume(menuTaskHandle);
              waitForInput = true;
              oledMana.enable();
              setLedIntensity(2);
              showCurrentTime();
            }
            Serial.println("resetting sleep timer");
            delay(10);

            wakeupTime = now;
          }

          if (waitForInput == false)
          {
            if (now - wakeupTime >= TIMER_WAKUP_TIME)
            {
              Serial.println("No input with timer going back to sleep");
              waitForInput = false;
              enableSleep();
            }
          }
          else
          {
            if (now - wakeupTime >= GPIO_WAKUP_TIME)
            {
              Serial.println("No input with input going back to sleep");
              waitForInput = false;
              enableSleep();
            }
          }
        }
        if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_TOUCHPAD)
        {
          if (!wokeUp)
          {
            Serial.println("Woke up, waiting for input...");
            wokeUp = true;
            wakeupTime = now;
            inputDetected = true;
          }

          if ((useAllButtons() != None || useAllTouch().touched == true || inputDetected == true) || ringing == true)
          {
            inputDetected = false;
            if (waitForInput == false)
            {
              Serial.println("Input or Alarm detected waiting more in touch");
              vTaskResume(menuTaskHandle);
              waitForInput = true;
              oledMana.enable();
              setLedIntensity(2);
              showCurrentTime();
            }
            Serial.println("resetting sleep timer");
            delay(10);

            wakeupTime = now;
          }

          if (now - wakeupTime >= GPIO_WAKUP_TIME)
          {
            Serial.println("No input after input going back to sleep");
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
  esp_sleep_enable_ext1_wakeup((1ULL << TOUCH_INTERRUPT) | (1ULL << MCP_INTERRUPT_PIN), ESP_EXT1_WAKEUP_ANY_LOW);
  esp_sleep_enable_timer_wakeup(SLEEPING_TIME);
}

void enableSleep()
{

  delay(500);
  vTaskSuspend(menuTaskHandle);
  oledMana.disable();
  oledMana.disable(); // Just to make sure because manager can take a bit before reacting if many write operations are ordered
  delay(500);
  setLedIntensity(0);
  delay(100);
  LedDisplay.clear();
  initSleep();
  wokeUp = false;
  esp_err_t sleep_result = esp_light_sleep_start();

  if (sleep_result == ESP_OK)
  {
    Serial.println("Light sleep entered and woke up successfully.");
  }
  else if (sleep_result == ESP_ERR_SLEEP_REJECT)
  {
    Serial.println("Sleep request rejected: Wakeup source set before the sleep request.");
  }
  else if (sleep_result == ESP_ERR_SLEEP_TOO_SHORT_SLEEP_DURATION)
  {
    Serial.println("Sleep duration too short: Minimum sleep duration not met.");
  }
  else
  {
    Serial.printf("Unexpected sleep error: %d\n", sleep_result);
  }
  // setTouchInterrupt(TOUCH_1_Seg_PIN, TOUCH_1_Seg_THRESHOLD_BAT);
  // setTouchInterrupt(TOUCH_2_Seg_PIN, TOUCH_2_Seg_THRESHOLD_BAT);
  // setTouchInterrupt(TOUCH_3_Seg_PIN, TOUCH_3_Seg_THRESHOLD_BAT);
  // setTouchInterrupt(TOUCH_4_Seg_PIN, TOUCH_4_Seg_THRESHOLD_BAT);
  // setTouchInterrupt(TOUCH_5_Seg_PIN, TOUCH_5_Seg_THRESHOLD_BAT);
  syncTimeLibWithRTC();
  checkAlarms();
  delay(200);
}

double readVoltage(byte pin)
{
  double reading = analogRead(VOLTAGE_DIVIDER_PIN);
  if (reading < 1 || reading > 4095)
    return 0;
  double voltage = -0.000000000000016 * pow(reading, 4) + 0.000000000118171 * pow(reading, 3) - 0.000000301211691 * pow(reading, 2) + 0.001109019271794 * reading + 0.034143524634089;
  return voltage * 1000;
}

float getBatteryVoltage()
{
  double miliVolts = readVoltage(VOLTAGE_DIVIDER_PIN);
  miliVolts = miliVolts - ADC_OFFSET;
  float batteryVoltage = miliVolts / ADC_VOLTAGE_DIVIDER;

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
    return percentage;
}