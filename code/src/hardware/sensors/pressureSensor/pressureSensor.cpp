#include "pressureSensor.h"

Adafruit_BMP280 bmp;
static bool bmpInitialized = false;

// Private helper to configure BMP280 settings without code duplication
static void configureBMPSensor()
{
    bmp.setSampling(Adafruit_BMP280::MODE_FORCED,
                    Adafruit_BMP280::SAMPLING_X2,
                    Adafruit_BMP280::SAMPLING_X16,
                    Adafruit_BMP280::FILTER_X16,
                    Adafruit_BMP280::STANDBY_MS_500);
}

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

    float temperature = -999.0f;

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
    unsigned long previousMillisChart = 0;
    while (true)
    {
        unsigned long currentMillis = millis();
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
        pressureTask,
        "PressureTask",
        2048,
        NULL,
        1,
        NULL
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
            configureBMPSensor();
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

void disablePressureSensor()
{
    if (!bmpInitialized) return;

    if (lockI2C())
    {
        bmp.setSampling(Adafruit_BMP280::MODE_SLEEP);
        unlockI2C();
    }

    Serial.println(F("BMP280 placed into sleep mode."));
}

void enablePressureSensor()
{
    if (!bmpInitialized) return;

    if (lockI2C())
    {
        configureBMPSensor();
        unlockI2C();
    }

    Serial.println(F("BMP280 woken up and restored to forced mode."));
}