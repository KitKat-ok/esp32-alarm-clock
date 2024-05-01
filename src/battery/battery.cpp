#include "battery.h"

float voltageSamples[NUM_SAMPLES];
int sampleIndex = 0;

int batteryPercentage;

float charging = false;

bool wentToSleep = false;

void goToSleep();
void initSleep();

void initBattery()
{
  for (int i = 0; i < NUM_SAMPLES; ++i)
  {
    float rawVoltage = analogRead(VOLTAGE_DIVIDER_PIN) * (3.3 / 4095.0) + 0.8;
    voltageSamples[i] = rawVoltage;
  }
}

void manageBattery(void *parameter);

void createBatteryTask()
{
  xTaskCreatePinnedToCore(
      manageBattery, // Function to implement the task
      "Battery",     // Task name
      10000,         // Stack size (words)
      NULL,          // Task input parameter
      2,             // Priority (0 is lowest)
      NULL,          // Task handle
      0              // Core to run the task on (0 or 1)
  );
}

void manageBattery(void *parameter)
{
  while (true)
  {
    LowBattery();
    batteryPercentage = getBatteryPercentage();

    int chargingState = analogRead(CHARGING_PIN);
    int standbyState = analogRead(FULLY_CHARGED_PIN);
    Serial.print("Charging State: ");
    Serial.println(chargingState);

    Serial.print("Standby State: ");
    Serial.println(standbyState);
    Serial.println("WiFi status " + wifiStatusToString(WiFi.status()));

    eTaskState WiFiTaskState = eTaskGetState(wifiTask);
    WiFiTaskState = eTaskGetState(wifiTask);
    if (standbyState > STANDBY_THRESHOLD || chargingState > CHARGING_THRESHOLD)
    {
      charging = true;
      wentToSleep = false;
      Serial.println("charging");
      vTaskDelay(pdMS_TO_TICKS(500));
      if (!WiFi.isConnected() && WiFiTaskRunning == false)
      {
        Serial.println("launching WiFi task");
        createWifiTask();
      }
      if (dimmingTaskRunning == false)
      {
        createDimmingTask();
        vTaskDelay(5000);
      }
    }
    else
    {
      charging = false;

      turnOffWifi();
      vTaskDelay(pdMS_TO_TICKS(500));
      while (wentToSleep == false)
      {
        vTaskDelay(pdMS_TO_TICKS(10));
        Serial.println("Going to sleep in 50 seconds");
        for (int i = 0; i < 50; i++)
        {
          chargingState = analogRead(CHARGING_PIN);
          standbyState = analogRead(FULLY_CHARGED_PIN);
          vTaskDelay(pdMS_TO_TICKS(1000)); // Delay for 1 second
          if ((standbyState > STANDBY_THRESHOLD) || (chargingState > CHARGING_THRESHOLD))
          {
            Serial.println("Charger Connected not going to sleep");
            vTaskDelay(pdMS_TO_TICKS(100));
            break; // Break the loop if the condition is met
          }
        }
        if ((standbyState > STANDBY_THRESHOLD) || (chargingState > CHARGING_THRESHOLD))
        {
          break; // Break the loop if the condition is met
        }
        if (dimmingTaskRunning == true)
        {
          vTaskDelay(pdMS_TO_TICKS(10));
          vTaskDelete(dimmingTask);
          dimmingTaskRunning = false;
        }
        Serial.println("Going to sleep");
        vTaskDelay(pdMS_TO_TICKS(200));
        wentToSleep = true;
        goToSleep();
      }

      if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_TIMER)
      {
        Serial.println("Woke up from Timer");
        display.dim(true);
        LedDisplay.clear();
        static unsigned long startTime = millis();
        if (millis() - startTime >= TIMER_WAKUP_TIME)
        {
          LedDisplay.clear();
          Serial.println("Going to sleep");
          vTaskDelay(pdMS_TO_TICKS(200));
          goToSleep();
          startTime = millis();
        }
      }

      if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_TOUCHPAD)
      {
        Serial.println("Woke up from touch button");
        display.ssd1306_command(SSD1306_DISPLAYON);
        display.dim(false);
        int currentHour = hour();
        int currentMinute = minute();
        if (currentHour >= 23 || currentHour < 10)
        {
          LedDisplay.setBrightness(2);
          LedDisplay.showNumberDecEx(currentHour * 100 + currentMinute, 0b11100000, true);
          while (maxBrightness == true)
          {
            vTaskDelay(10);
          }
          dimLedDisplay();
          LedDisplay.showNumberDecEx(currentHour * 100 + currentMinute, 0b11100000, true);
        }
        else
        {
          LedDisplay.setBrightness(7);
          LedDisplay.showNumberDecEx(currentHour * 100 + currentMinute, 0b11100000, true);
        }
        static unsigned long startTime = millis();

        if (millis() - startTime >= GPIO_WAKUP_TIME)
        {
          Serial.println("Going to sleep");
          vTaskDelay(pdMS_TO_TICKS(200));
          goToSleep();
          startTime = millis();
        }
      }
      Serial.println("Not charging");
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void initSleep()
{
  esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);

  esp_sleep_enable_timer_wakeup(SLEEPING_TIME);

  touchSleepWakeUpEnable(TOUCH_BUTTON_PIN, TOUCH_BUTTON_THRESHOLD_ON_BATTERY);
}

void goToSleep()
{
  display.ssd1306_command(SSD1306_DISPLAYOFF);

  display.dim(true);
  display.display();
  LedDisplay.clear();

  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);

  initSleep();
  esp_light_sleep_start();
}

int getBatteryPercentage()
{
  // Read the raw voltage
  float rawVoltage = analogRead(VOLTAGE_DIVIDER_PIN) * (3.30 / 4095.00) + 0.9;
  Serial.println("Battery voltage: " + String(rawVoltage));

  // Store the new voltage in the array
  voltageSamples[sampleIndex] = rawVoltage;

  // Increment the sample index and wrap around if necessary
  sampleIndex = (sampleIndex + 1) % NUM_SAMPLES;

  // Calculate the average of the stored voltages
  float smoothedVoltage = 0.00;
  for (int i = 0; i < NUM_SAMPLES; i++)
  {
    smoothedVoltage += voltageSamples[i];
  }
  smoothedVoltage /= NUM_SAMPLES;

  // Calculate and return the battery percentage
  int percentage = ((smoothedVoltage - MIN_VOLTAGE) / (MAX_VOLTAGE - MIN_VOLTAGE)) * 100.00;
  if (percentage < 0)
  {
    percentage = 0;
    return percentage;
  }
  else
    return percentage;
}