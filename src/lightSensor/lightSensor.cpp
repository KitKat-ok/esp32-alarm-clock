#include "defines.h"

void dimmingFunction(void *pvParameters);
void dimOledDisplay();
void dimLedDisplay();

void createDimmingTask()
{
    xTaskCreatePinnedToCore(
        dimmingFunction, /* Task function. */
        "DimTask",       /* String with name of task. */
        10000,           /* Stack size in words. */
        NULL,            /* Parameter passed as input of the task */
        1,               /* Priority of the task. */
        NULL,            /* Task handle. */
        0                /* Core where the task should run. */
    );
}

void dimmingFunction(void *pvParameters)
{
    for (;;)
    {
        int currentHour = hour();
        int currentMinute = minute();
        // Call the functions to dim displays
        dimOledDisplay();
        dimLedDisplay();
        Serial.println("Reading brightness and dimming displays accordingly");
        LedDisplay.showNumberDecEx(currentHour * 100 + currentMinute, 0b11100000, true);

        vTaskDelay(pdMS_TO_TICKS(10000)); // 30 seconds delay
    }
}

void dimOledDisplay()
{
    if (lightMeter.readLightLevel() > OLED_DIM_THRESHOLD)
    {
        display.dim(false);
    }
    else
    {
        display.dim(true);
    }
}

void dimLedDisplay()
{
    int lightLevel = lightMeter.readLightLevel();
    if (lightLevel > LED_DIM_THRESHOLD)
    {
        LedDisplay.setBrightness(map(constrain(lightLevel, 0, LED_DIM_THRESHOLD), 0, LED_DIM_THRESHOLD, 0, 7));
        Serial.println("Brightness of Led display " + String(map(constrain(lightLevel, 0, LED_DIM_THRESHOLD), 0, LED_DIM_THRESHOLD, 0, 7)));
    }
    else
    {
        LedDisplay.setBrightness(0);
    }
}