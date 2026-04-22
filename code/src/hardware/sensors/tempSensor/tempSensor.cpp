#include "tempSensor.h"

Adafruit_SHT4x sht4 = Adafruit_SHT4x();

float readTemperature()
{
    sensors_event_t humidity, temp;

    sht4.getEvent(&humidity, &temp); // Populate temp and humidity objects with fresh data
    return temp.temperature;
}

float readHumidity()
{
    sensors_event_t humidity, temp;

    sht4.getEvent(&humidity, &temp); // Populate temp and humidity objects with fresh data
    return humidity.relative_humidity;
}


void tempTask(void *pvParameters)
{
    unsigned long previousMillisChart = 0;     // Will store the last time the function was called
    while (true)
    {
        unsigned long currentMillis = millis(); // Get the current time
        if (currentMillis - previousMillisChart >= INTERVAL_CHARTS)
        {
            for (int i = 0; i < CHART_READINGS - 1; i++)
            {
                temperatureArray[i] = temperatureArray[i + 1];
                humidityArray[i] = humidityArray[i + 1];
            }

            temperatureArray[CHART_READINGS - 1] = readTemperature();
            humidityArray[CHART_READINGS - 1] = readHumidity();
            previousMillisChart = currentMillis;
        }
        vTaskDelay(pdMS_TO_TICKS(600000));
    }
}

void createTempTask()
{
    Serial.println("Creating tempTask");

    xTaskCreate(
        tempTask, /* Task function */
        "TempTask",     /* String with name of task */
        2048,          /* Stack size in words */
        NULL,           /* Parameter passed as input of the task */
        1,              /* Priority of the task */
        NULL           /* Task handle */
    );
}

void initTempSensor()
{
  if (!sht4.begin())
  {
    Serial.println("Couldn't find SHT4x");
  }
  else
  {
    Serial.println("Found SHT4x sensor");
  }
  sht4.setPrecision(SHT4X_HIGH_PRECISION);
  sht4.setHeater(SHT4X_NO_HEATER);
}