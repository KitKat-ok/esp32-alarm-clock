#include "tempSensor.h"

float temperature = 0.0;

float readTemperature()
{
  sensors_event_t humidity, temp;
  
  uint32_t timestamp = millis();
  sht4.getEvent(&humidity, &temp);// populate temp and humidity objects with fresh data
  timestamp = millis() - timestamp;

  Serial.print("Temperature: "); Serial.print(temp.temperature); Serial.println(" degrees C");
  Serial.print("Humidity: "); Serial.print(humidity.relative_humidity); Serial.println("% rH");

  Serial.print("Read duration (ms): ");
  Serial.println(timestamp);
  return temp.temperature;
}

const unsigned long intervalTemp = 300000; 
unsigned long previousMillisTemp = 0;     // Will store last time the function was called

void setTemperature(void *pvParameters)
{
    while (true)
    {
        unsigned long currentMillis = millis(); // Get the current time
        temperature = readTemperature() - TEMP_OFFSET;
        if (currentMillis - previousMillisTemp >= intervalTemp)
        {
            for (int i = 0; i < TEMP_CHART_READINGS - 1; i++)
            {
                temperatureArray[i] = temperatureArray[i + 1];
            }

            // Add the new reading to the end of the array
            temperatureArray[TEMP_CHART_READINGS - 1] = temperature; // Replace with your temperature reading function
            previousMillisTemp = currentMillis;
        }
        vTaskDelay(1000);
    }
}

void createTempTask()
{
    Serial.print("creating tempTask");

    temperature = readTemperature() - TEMP_OFFSET;

    for (int i = 0; i < TEMP_CHART_READINGS - 1; i++)
    {
        temperatureArray[i] = temperatureArray[i + 1];
    }

    temperatureArray[TEMP_CHART_READINGS - 1] = temperature; 
    xTaskCreatePinnedToCore(
        setTemperature, /* Task function. */
        "DimTask",      /* String with name of task. */
        10000,          /* Stack size in words. */
        NULL,           /* Parameter passed as input of the task */
        1,              /* Priority of the task. */
        NULL,           /* Task handle. */
        1               /* Core where the task should run. */
    );
}