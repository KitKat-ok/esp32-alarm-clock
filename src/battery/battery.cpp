#include "battery.h"

float voltageSamples[NUM_SAMPLES];
int sampleIndex = 0;

int batteryPercentage;

float charging = false;


void initBattery() {
  pinMode(CHARGING_PIN, INPUT);
  pinMode(FULLY_CHARGED_PIN, INPUT);
   for (int i = 0; i < NUM_SAMPLES; ++i) {
    float rawVoltage = analogRead(VOLTAGE_DIVIDER_PIN) * (3.3 / 4095.0) + 0.8;
    voltageSamples[i] = rawVoltage;
  }
}

void manageBattery(void *parameter);

void createBatteryTask() {
    xTaskCreatePinnedToCore(
    manageBattery,          // Function to implement the task
    "Battery",        // Task name
    10000,           // Stack size (words)
    NULL,            // Task input parameter
    1,               // Priority (0 is lowest)
    NULL,   // Task handle
    0                // Core to run the task on (0 or 1)
  );
}



String wifiStatusToString(int status) {
  switch (status) {
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


void manageBattery(void *parameter) {
  while (true)
  {
     int chargingState = digitalRead(CHARGING_PIN);
    int standbyState = digitalRead(FULLY_CHARGED_PIN);
    eTaskState taskState = eTaskGetState(wifiTask);
    if (standbyState == HIGH || chargingState == HIGH)
    {
      charging = true;
      Serial.println("charging");
      Serial.println(wifiStatusToString(WiFi.status()));
      if (!WiFi.isConnected() && WiFiTaskRunning == false)
      {
        Serial.println("launching WiFi task");
        createWifiTask();
      }
    } else {
      charging = false;
        turnOffWifi();
      Serial.println("Not charging");
    }
    
    batteryPercentage = getBatteryPercentage();
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}


int getBatteryPercentage() {
  // Read the raw voltage
  float rawVoltage = analogRead(VOLTAGE_DIVIDER_PIN) * (3.3 / 4095.0) + 0.8;

  // Store the new voltage in the array
  voltageSamples[sampleIndex] = rawVoltage;

  // Increment the sample index and wrap around if necessary
  sampleIndex = (sampleIndex + 1) % NUM_SAMPLES;

  // Calculate the average of the stored voltages
  float smoothedVoltage = 0;
  for (int i = 0; i < NUM_SAMPLES; i++) {
    smoothedVoltage += voltageSamples[i];
  }
  smoothedVoltage /= NUM_SAMPLES;

  // Calculate and return the battery percentage
  int percentage = ((smoothedVoltage - MIN_VOLTAGE) / (MAX_VOLTAGE - MIN_VOLTAGE)) * 100.0;
  if (percentage < 0)
  {
    percentage = 0;
    return percentage;
  } else 
  return percentage;
  
}