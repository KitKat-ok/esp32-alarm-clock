#include "mainPage.h"

float voltageReadings[NUM_READINGS];
int currentIndex = 0;
float smoothedVoltage = 0.0;

float readTemperature()
{
    int sensorValue = analogRead(TEMP_SENS_PIN);

    // Convert the analog reading to voltage
    float voltage = (sensorValue / 4095.0) * VOLTAGE_REFERENCE;

    // Fixed average using a circular buffer for voltage
    voltageReadings[currentIndex] = voltage;
    currentIndex = (currentIndex + 1) % NUM_READINGS;

    float fixedAverageVoltage = 0;
    for (int i = 0; i < NUM_READINGS; ++i)
    {
        fixedAverageVoltage += voltageReadings[i];
    }
    fixedAverageVoltage /= NUM_READINGS;

    // Exponential moving average for voltage
    smoothedVoltage = (SMOOTHING_FACTOR * voltage) + ((1.0 - SMOOTHING_FACTOR) * smoothedVoltage);

    // Combine both readings for a smoother output
    float combinedVoltage = (fixedAverageVoltage + smoothedVoltage) / 2.0;

    // Convert smoothed voltage to temperature in degrees Celsius
    float temperatureC = (combinedVoltage - 0.375);
    temperatureC = temperatureC / 0.01;

    return temperatureC;
}

void initTemperature()
{
    for (int i = 0; i < NUM_READINGS; ++i)
    {
        int sensorValue = analogRead(TEMP_SENS_PIN);
        sensorValue = analogRead(TEMP_SENS_PIN);

        // Convert the analog reading to voltage
        float voltage = (sensorValue / 4095.0) * VOLTAGE_REFERENCE;
        voltage = (sensorValue / 4095.0) * VOLTAGE_REFERENCE;
        voltageReadings[i] = voltage;
    }
}

void showMainPage()
{
    display.clearDisplay();
    display.drawRect(0, SCREEN_HEIGHT / 3 - 5, SCREEN_WIDTH, 2, SSD1306_WHITE);
    display.setTextSize(1);
    display.setFont(&DejaVu_LGC_Sans_15);
    centerText(String(day()) + "/" + String(month()) + "/" + String(year()), SCREEN_HEIGHT / 2);
    display.setTextSize(1);
    display.setFont(&DejaVu_LGC_Sans_Bold_10);
    centerText(String(batteryPercentage) + "%", SCREEN_HEIGHT / 3 - 7);
    centerText(getWeekdayName(), SCREEN_HEIGHT / 2 + 10);
    centerText("Temp: " + String(readTemperature()) + " C", SCREEN_HEIGHT / 2 + 20);
    display.display();
}
