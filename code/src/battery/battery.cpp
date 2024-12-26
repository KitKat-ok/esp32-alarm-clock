#include "battery.h"

int batteryPercentage;

bool powerConnected = false;
bool charging = false;

bool wentToSleep = false;

bool goToSleep = false;

void enableSleep();
void initSleep();

void manageBattery(void *parameter);

void controlCharger();

void createBatteryTask()
{
  xTaskCreatePinnedToCore(
      manageBattery, // Function to implement the task
      "Battery",     // Task name
      10000,         // Stack size (words)
      NULL,          // Task input parameter
      2,             // Priority (0 is lowest)
      NULL,          // Task handle
      1              // Core to run the task on (0 or 1)
  );
}

bool checkCharging()
{
  int chargingState = digitalRead(POWER_STATE_PIN);
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

void manageBattery(void *parameter)
{
  while (true)
  {
    batteryPercentage = getBatteryPercentage();

    Serial.println("WiFi status " + wifiStatusToString(WiFi.status()));

    eTaskState WiFiTaskState = eTaskGetState(wifiTask);
    checkCharging();
    if (powerConnected == true)
    {
      wentToSleep = false;
      Serial.println("Power connected");
      controlCharger();
      vTaskDelay(pdMS_TO_TICKS(500));
      if (!WiFi.isConnected() && WiFiTaskRunning == false)
      {
        Serial.println("launching WiFi task");
        createWifiTask();
      }
    }
    else
    {
      digitalWrite(CHARGER_CONTROL_PIN, LOW); // Disable Charger when power is off
      turnOffWifi();
      vTaskDelay(pdMS_TO_TICKS(500));

      static unsigned long sleepStartTime = 0;
      static bool preparingForSleep = false;

      if (preparingForSleep == false)
      {
        Serial.println("Preparing to go to sleep in 10 seconds");
        sleepStartTime = millis();
        preparingForSleep = true;
      }

      if (preparingForSleep == false)
      {
        if (millis() - sleepStartTime >= 10000)
        {
          if (powerConnected == true)
          {
            Serial.println("Power Connected, canceling sleep preparation");
            preparingForSleep = false;
          }
          else
          {
            Serial.println("Requesting Sleep");
            wentToSleep = true;
            enableSleep();
          }
        }
      }
      if (wentToSleep == true && powerConnected == true)
      {

        // Handle wakeup cases
        if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_TIMER)
        {
          Serial.println("Woke up from Timer");
          LedDisplay.clear();
          int currentHour = hour();
          int currentMinute = minute();
          static unsigned long startTime = millis();
          if (checkForInput())
          {
            manager.oledEnable();
            LedDisplay.setBrightness(7);
            LedDisplay.showNumberDecEx(currentHour * 100 + currentMinute, 0b11100000, true);
          }

          if (millis() - startTime >= TIMER_WAKUP_TIME && !checkCharging() && inputDetected == false)
          {
            LedDisplay.clear();
            vTaskDelay(pdMS_TO_TICKS(200));
            enableSleep();
            startTime = millis();
          }
        }

        unsigned long delayDuration = 30000;

        if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_TOUCHPAD && goToSleep == false)
        {
          Serial.println("Woke up from touch button");
          manager.oledEnable();
          int currentHour = hour();
          int currentMinute = minute();

          LedDisplay.setBrightness(7);
          LedDisplay.showNumberDecEx(currentHour * 100 + currentMinute, 0b11100000, true);
          maxBrightness = true;
        }

        static unsigned long startTime = millis();
        if (millis() - startTime >= GPIO_WAKUP_TIME && inputDetected == false)
        {
          vTaskDelay(pdMS_TO_TICKS(200));
          enableSleep();
          startTime = millis();
        }
      }
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void controlCharger()
{
  float batteryVoltage = getBatteryVoltage();

  if (charging)
  {
    if (batteryVoltage >= BATT_TARGET_VOLTAGE)
    {
      charging = false;
      digitalWrite(CHARGER_CONTROL_PIN, LOW); // Stop charging
      Serial.println("Charging stopped (target voltage reached).");
    }
  }
  else
  {
    if (batteryVoltage <= (BATT_TARGET_VOLTAGE - BATT_HYSTERESIS))
    {
      charging = true;
      digitalWrite(CHARGER_CONTROL_PIN, HIGH); // Start charging
      Serial.println("Charging started (voltage dropped).");
    }
  }

  // Debugging: Always print battery voltage for monitoring
  Serial.print("Battery Voltage: ");
  Serial.print(batteryVoltage);
  Serial.print(" V - Charging: ");
  Serial.println(charging ? "ON" : "OFF");
}

// median

void initSleep()
{
  esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);

  esp_sleep_enable_timer_wakeup(SLEEPING_TIME);

  touchSleepWakeUpEnable(TOUCH_BUTTON_PIN, TOUCH_BUTTON_THRESHOLD_ON_BATTERY);

  esp_sleep_enable_touchpad_wakeup();
}

void enableSleep()
{
  goToSleep = true;
}

void listenToSleep()
{
  if (goToSleep == true)
  {
    initSleep();
    Serial.println("Initialized sleep");
    Serial.println("Light Sleep");
    Serial.flush();
    Serial.println("Going to sleep");
    maxBrightness = false;
    inputDetected = false;
    turnOffWifiMinimal();
    manager.oledFadeOut();
    delay(500);
    manager.oledDisable();
    display.ssd1306_command(SSD1306_DISPLAYOFF); // Just to make sure because manager can take a bit before reacting if many write operations are ordered
    delay(500);
    LedDisplay.clear();
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    esp_err_t sleep_result = esp_light_sleep_start();

    // Check the result of the sleep request
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
    goToSleep = false;
  }
}

#include <MedianFilterLib2.h> // Include the Median Filter Library

#define MEDIAN_WINDOW_SIZE 10

MedianFilter2<float> medianFilter(MEDIAN_WINDOW_SIZE); // Instantiate Median Filter


double readVoltage(byte pin) {
  double reading = adc1_get_raw(ADC1_CHANNEL_6); 
  Serial.println("Raw adc reading" + String(reading));
  if (reading < 1 || reading > 4095) return 0;
  double voltage = -0.000000000000016 * pow(reading, 4) 
                   + 0.000000000118171 * pow(reading, 3)
                   - 0.000000301211691 * pow(reading, 2)
                   + 0.001109019271794 * reading 
                   + 0.034143524634089;
  return voltage * 1000;
}

float getBatteryVoltage()
{
  double miliVolts = readVoltage(VOLTAGE_DIVIDER_PIN);
  miliVolts = miliVolts - ADC_OFFSET;
  float batteryVoltage = miliVolts / ADC_VOLTAGE_DIVIDER;

  // Add the value to the median filter and get the filtered result
  float medianVoltage = medianFilter.AddValue(batteryVoltage);

  Serial.print("Battery voltage (median): ");
  Serial.println(medianVoltage, 3); // 3 decimal places
  Serial.print("Raw Voltage Divider mV: ");
  Serial.println(miliVolts, 3);

  return medianVoltage;
}

int getBatteryPercentage()
{
  int percentage = ((getBatteryVoltage() - MIN_VOLTAGE) / (MAX_VOLTAGE - MIN_VOLTAGE)) * 100.00;
  percentage = min(percentage, 100);
  if (percentage < 0)
  {
    percentage = 0;
    return percentage;
  }
  else
    return percentage;
}