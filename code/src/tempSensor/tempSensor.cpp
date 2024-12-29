#include "tempSensor.h"
float readTemperature()
{
    sensors_event_t humidity, temp;

    uint32_t timestamp = millis();
    sht4.getEvent(&humidity, &temp); // Populate temp and humidity objects with fresh data
    timestamp = millis() - timestamp;

    Serial.print("Temperature: ");
    Serial.print(temp.temperature);
    Serial.println(" degrees C");

    Serial.print("Read duration (ms): ");
    Serial.println(timestamp);
    return temp.temperature;
}

float readHumidity()
{
    sensors_event_t humidity, temp;

    uint32_t timestamp = millis();
    sht4.getEvent(&humidity, &temp); // Populate temp and humidity objects with fresh data
    timestamp = millis() - timestamp;

    Serial.print("Humidity: ");
    Serial.print(humidity.relative_humidity);
    Serial.println("% rH");

    Serial.print("Read duration (ms): ");
    Serial.println(timestamp);
    return humidity.relative_humidity;
}

unsigned long previousMillisTemp = 0;     // Will store the last time the function was called

void setTemperature(void *pvParameters)
{
    while (true)
    {
        unsigned long currentMillis = millis(); // Get the current time
        if (currentMillis - previousMillisTemp >= INTERVAL_CHARTS)
        {
            // Shift temperature readings
            for (int i = 0; i < CHART_READINGS - 1; i++)
            {
                temperatureArray[i] = temperatureArray[i + 1];
                humidityArray[i] = humidityArray[i + 1];
            }

            // Add new readings to the end of the arrays
            temperatureArray[CHART_READINGS - 1] = readTemperature();
            humidityArray[CHART_READINGS - 1] = readHumidity();
            previousMillisTemp = currentMillis;
        }
        vTaskDelay(1000);
    }
}

void createTempTask()
{
    Serial.println("Creating tempTask");
    // Initialize arrays with the first readings
    for (int i = 0; i < CHART_READINGS; i++)
    {
        temperatureArray[i] = readTemperature();
        humidityArray[i] = readHumidity();
    }

    xTaskCreatePinnedToCore(
        setTemperature, /* Task function */
        "TempTask",     /* String with name of task */
        10000,          /* Stack size in words */
        NULL,           /* Parameter passed as input of the task */
        1,              /* Priority of the task */
        NULL,           /* Task handle */
        1               /* Core where the task should run */
    );
}
