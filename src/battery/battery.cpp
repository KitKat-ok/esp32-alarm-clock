#include "battery.h"

float voltageSamples[NUM_SAMPLES];
int sampleIndex = 0;

int batteryPercentage;


void initBattery() {
   for (int i = 0; i < NUM_SAMPLES; ++i) {
    voltageSamples[i] = MIN_VOLTAGE;
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


void manageBattery(void *parameter) {
  while (1)
  {
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