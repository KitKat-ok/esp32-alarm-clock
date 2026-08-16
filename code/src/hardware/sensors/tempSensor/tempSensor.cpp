#include "tempSensor.h"

Adafruit_SHT4x sht4 = Adafruit_SHT4x();
static bool shtInitialized = false;

// Helper to keep sensor settings in a single place
static void applyTempSensorSettings()
{
    sht4.setPrecision(SHT4X_HIGH_PRECISION);
    sht4.setHeater(SHT4X_NO_HEATER);
}

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
    unsigned long previousMillisChart = 0;
    while (true)
    {
        unsigned long currentMillis = millis();
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
        tempTask,
        "TempTask",
        2048,
        NULL,
        1,
        NULL
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
            applyTempSensorSettings();
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

void disableTempSensor()
{
    if (!shtInitialized) return;

    if (lockI2C())
    {
        sht4.reset();
        unlockI2C();
    }

    Serial.println("SHT4x placed into low-power idle mode.");
}

void enableTempSensor()
{
    if (!shtInitialized) return;

    if (lockI2C())
    {
        applyTempSensorSettings();
        unlockI2C();
    }

    Serial.println("SHT4x ready for operation.");
}