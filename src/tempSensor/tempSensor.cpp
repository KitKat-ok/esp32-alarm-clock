#include "tempSensor.h"

float temperature = 0.0;

Smoothed<float> tempSensor;

float readTemperature()
{
    tempSensor.add(analogReadMilliVolts(TEMP_SENS_PIN));
    int sensorValue = tempSensor.get();
    Serial.println("Smotthened reading: " + String(sensorValue));
    Serial.println("Bare reading: " + String(analogRead(TEMP_SENS_PIN)));

    float voltage = (sensorValue / 1000.0) - 0.037;
    Serial.println(String(voltage, 3));

    float temperatureC = voltage - 0.5;
    temperatureC = temperatureC / 0.01;

    return temperatureC;
}

const unsigned long intervalTemp = 60000; // Interval in milliseconds (30 seconds)
unsigned long previousMillisTemp = 0;     // Will store last time the function was called

void setTemperature(void *pvParameters)
{
    while (true)
    {
        unsigned long currentMillis = millis(); // Get the current time
        temperature = readTemperature();
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
    tempSensor.begin(SMOOTHED_AVERAGE, TEMP_SMOOTH_FACTOR);
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