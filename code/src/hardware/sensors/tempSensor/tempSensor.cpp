#include "tempSensor.h"

Adafruit_SHT4x sht4 = Adafruit_SHT4x();
static bool shtInitialized = false;

float readTemperature()
{
    if (!shtInitialized) return -999.0f;

    float temperature = -999.0f;
    sensors_event_t humidity, temp;

    if (lockI2C())
    {
        if (sht4.getEvent(&humidity, &temp))
        {
            temperature = temp.temperature;
        }
        unlockI2C();
    }

    return temperature;
}

float readHumidity()
{
    if (!shtInitialized) return -1.0f;

    float relative_humidity = -1.0f;
    sensors_event_t humidity, temp;

    if (lockI2C())
    {
        if (sht4.getEvent(&humidity, &temp))
        {
            relative_humidity = humidity.relative_humidity;
        }
        unlockI2C();
    }

    return relative_humidity;
}

void tempTask(void *pvParameters)
{
    unsigned long previousMillisChart = 0; // Will store the last time the function was called
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
        tempTask,   /* Task function */
        "TempTask", /* String with name of task */
        2048,       /* Stack size in words */
        NULL,       /* Parameter passed as input of the task */
        1,          /* Priority of the task */
        NULL        /* Task handle */
    );
}

void initTempSensor()
{
    bool initialized = false;

    if (lockI2C())
    {
        initialized = sht4.begin();
        if (initialized)
        {
            sht4.setPrecision(SHT4X_HIGH_PRECISION);
            sht4.setHeater(SHT4X_NO_HEATER);
        }
        unlockI2C();
    }

    if (!initialized)
    {
        Serial.println("Couldn't find SHT4x sensor! Ignoring and continuing...");
        shtInitialized = false;
        return;
    }

    shtInitialized = true;
    Serial.println("Found SHT4x sensor successfully.");
}