#include "pressureSensor.h"

Adafruit_BMP280 bmp;
static bool bmpInitialized = false;

float readAltitude()
{
    if (!bmpInitialized) return -1.0f;

    float altitude = -1.0f;

    if (lockI2C())
    {
        bmp.takeForcedMeasurement();
        if (isWeatherAvailable)
        {
            altitude = bmp.readAltitude(currentWeatherData.pressure);
        }
        unlockI2C();
    }

    return altitude;
}

float readTemperatureBMP()
{
    if (!bmpInitialized) return -999.0f;

    float temperature = -999.0f; // Return sentinel value on failure/timeout

    if (lockI2C())
    {
        temperature = bmp.readTemperature();
        unlockI2C();
    }

    return temperature;
}

float readPressure()
{
    if (!bmpInitialized) return 0.0f;

    float pressure = 0.0f;

    if (lockI2C())
    {
        bmp.takeForcedMeasurement();
        pressure = bmp.readPressure() * 0.01f;
        unlockI2C();
    }

    return pressure;
}

void pressureTask(void *pvParameters)
{
    unsigned long previousMillisChart = 0; // Will store the last time the function was called
    while (true)
    {
        unsigned long currentMillis = millis(); // Get the current time
        if (currentMillis - previousMillisChart >= INTERVAL_CHARTS)
        {
            for (int i = 0; i < CHART_READINGS - 1; i++)
            {
                pressureArray[i] = pressureArray[i + 1];
            }

            pressureArray[CHART_READINGS - 1] = readPressure();
            previousMillisChart = currentMillis;
        }
        vTaskDelay(pdMS_TO_TICKS(600000));
    }
}

void createPressureTask()
{
    Serial.println("Creating tempTask");

    xTaskCreate(
        pressureTask,   /* Task function */
        "PressureTask", /* String with name of task */
        2048,           /* Stack size in words */
        NULL,           /* Parameter passed as input of the task */
        1,              /* Priority of the task */
        NULL            /* Task handle */
    );
}

void initPressureSensor()
{
    bool initialized = false;

    if (lockI2C())
    {
        initialized = bmp.begin(BMP280_ADDRESS_ALT);

        if (initialized)
        {
            /* Default settings from datasheet. */
            bmp.setSampling(Adafruit_BMP280::MODE_FORCED,     /* Operating Mode. */
                            Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                            Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                            Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                            Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
        }
        unlockI2C();
    }

    if (!initialized)
    {
        Serial.println(F("Could not find a valid BMP280 sensor! Ignoring and continuing..."));
        bmpInitialized = false;
        return;
    }

    bmpInitialized = true;
    Serial.println(F("BMP280 initialized successfully."));
}