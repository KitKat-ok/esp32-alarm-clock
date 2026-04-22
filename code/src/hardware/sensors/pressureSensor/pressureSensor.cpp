#include "pressureSensor.h"

Adafruit_BMP280 bmp;

float readAltitude()
{
    float altitude;
    if (isWeatherAvailable)
    {
        altitude = bmp.readAltitude(currentWeatherData.pressure);
    }
    else
    {
        altitude = 0;
    }

    return altitude;
}

float readTemperatureBMP()
{
    float temperature = bmp.readTemperature();
    return temperature;
}

float readPressure()
{
    float pressure = bmp.readPressure();
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
                temperatureArray[i] = temperatureArray[i + 1];
                humidityArray[i] = humidityArray[i + 1];
            }

            temperatureArray[CHART_READINGS - 1] = readTemperatureBMP();
            humidityArray[CHART_READINGS - 1] = readPressure();
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
    if (!bmp.begin(BMP280_ADDRESS_ALT))
    {
        Serial.println(F("Could not find a valid BMP280 sensor, check wiring or "
                         "try a different address!"));
        while (1)
            delay(10);
    }

    /* Default settings from datasheet. */
    bmp.setSampling(Adafruit_BMP280::MODE_FORCED,     /* Operating Mode. */
                    Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                    Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                    Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                    Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
}
