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

String wifiStatusToString(int status)
{
  switch (status)
  {
  case WL_IDLE_STATUS:
    return "Idle";
  case WL_NO_SSID_AVAIL:
    return "No SSID Available";
  case WL_SCAN_COMPLETED:
    return "Scan Completed";
  case WL_CONNECTED:
    return "Connected";
  case WL_CONNECT_FAILED:
    return "Connect Failed";
  case WL_CONNECTION_LOST:
    return "Connection Lost";
  case WL_DISCONNECTED:
    return "Disconnected";
  default:
    return "Unknown Status";
  }
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
      lightMeter.configure(BH1750::CONTINUOUS_HIGH_RES_MODE);
      Serial.println("charging");
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
      lightMeter.configure(BH1750::CONTINUOUS_LOW_RES_MODE);
      vTaskDelay(pdMS_TO_TICKS(500));
      while (wentToSleep == false)
      {
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
        LedDisplay.setBrightness(0);
        LedDisplay.showNumberDecEx(hour() * 100 + minute(), 0b11100000, true);
        static unsigned long startTime = millis();
        if (millis() - startTime >= TIMER_WAKUP_TIME)
        {
          LedDisplay.setBrightness(0);
          LedDisplay.showNumberDecEx(hour() * 100 + minute(), 0b11100000, true);
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
        LedDisplay.setBrightness(7);
        LedDisplay.showNumberDecEx(hour() * 100 + minute(), 0b11100000, true);
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
  LedDisplay.setBrightness(0);
  LedDisplay.showNumberDecEx(hour() * 100 + minute(), 0b11100000, true);

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