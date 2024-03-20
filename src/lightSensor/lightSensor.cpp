#include "defines.h"

void dimmingFunction(void *pvParameters);
void dimOledDisplay();
void dimLedDisplay();

bool dimmingTaskRunning = false;

TaskHandle_t dimmingTask;

void createDimmingTask()
{
    Serial.print("creating dimmingTask");
    xTaskCreatePinnedToCore(
        dimmingFunction, /* Task function. */
        "DimTask",       /* String with name of task. */
        10000,           /* Stack size in words. */
        NULL,            /* Parameter passed as input of the task */
        1,               /* Priority of the task. */
        &dimmingTask,    /* Task handle. */
        0                /* Core where the task should run. */
    );
}

const unsigned long intervalLight = 60000; // Interval in milliseconds (30 seconds)
unsigned long previousMillisLight = 0;     // Will store last time the function was called

void dimmingFunction(void *pvParameters)
{
    while (true)
    {
        unsigned long currentMillis = millis(); // Get the current time
        if (currentMillis - previousMillisLight >= intervalLight)
        {
            for (int i = 0; i < TEMP_CHART_READINGS - 1; i++)
            {
                lightArray[i] = lightArray[i + 1];
            }

            // Add the new reading to the end of the array
            lightArray[TEMP_CHART_READINGS - 1] = lightMeter.readLightLevel(); // Replace with your temperature reading function
            previousMillisLight = currentMillis;
        }
        dimmingTaskRunning = true;

        int currentHour = hour();
        int currentMinute = minute();
#define BUTTON_UP_PIN GPIO_NUM_12
#define BUTTON_DOWN_PIN GPIO_NUM_13
#define BUTTON_CONFIRM_PIN GPIO_NUM_14
#define BUTTON_EXIT_PIN GPIO_NUM_15

        if (touchRead(TOUCH_BUTTON_PIN) < TOUCH_BUTTON_THRESHOLD ||
            digitalRead(BUTTON_UP_PIN) == LOW ||
            digitalRead(BUTTON_DOWN_PIN) == LOW ||
            digitalRead(BUTTON_CONFIRM_PIN) == LOW ||
            digitalRead(BUTTON_EXIT_PIN) == LOW)
        {
            Serial.println("Button pressed");

            Serial.println("setting max brightness");
            display.ssd1306_command(SSD1306_DISPLAYON);
            display.dim(false);
            LedDisplay.setBrightness(7);
            LedDisplay.showNumberDecEx(currentHour * 100 + currentMinute, 0b11100000, true);
            vTaskDelay(pdMS_TO_TICKS(15000));
        }
        else
        {
            // Call the functions to dim displays
            currentHour = hour();
            currentMinute = minute();
            dimOledDisplay();
            dimLedDisplay();
            Serial.println("touch level: " + String(touchRead(TOUCH_BUTTON_PIN)));

            Serial.println("Reading brightness and dimming displays accordingly");
            LedDisplay.showNumberDecEx(currentHour * 100 + currentMinute, 0b11100000, true);
        }

        unsigned long startTime = millis();
        while ((millis() - startTime) < 3000)
        {
            if (touchRead(TOUCH_BUTTON_PIN) < TOUCH_BUTTON_THRESHOLD ||
                digitalRead(BUTTON_UP_PIN) == LOW ||
                digitalRead(BUTTON_DOWN_PIN) == LOW ||
                digitalRead(BUTTON_CONFIRM_PIN) == LOW ||
                digitalRead(BUTTON_EXIT_PIN) == LOW)
            {
                break;
            }
            vTaskDelay(pdMS_TO_TICKS(1));
        }
    }
}

void dimOledDisplay()
{
    if (lightMeter.readLightLevel() == 0.0)
    {
        display.dim(true);
        display.ssd1306_command(SSD1306_DISPLAYOFF);
        Serial.println("display off");
    }
    else if (lightMeter.readLightLevel() > OLED_DIM_THRESHOLD)
    {
        display.dim(false);
        display.ssd1306_command(SSD1306_DISPLAYON);
    }
    else
    {
        display.ssd1306_command(SSD1306_DISPLAYON);
        display.dim(true);
    }
}

void dimLedDisplay()
{
    int lightLevel = lightMeter.readLightLevel();
    if (lightLevel > LED_DIM_THRESHOLD)
    {
        LedDisplay.setBrightness(map(constrain(lightLevel, LED_DIM_THRESHOLD, 100), LED_DIM_THRESHOLD, 100, 0, 7));
        Serial.println("Brightness of Led display " + String(map(constrain(lightLevel, 0, LED_DIM_THRESHOLD), 0, LED_DIM_THRESHOLD, 0, 7)));
    }
    else
    {
        LedDisplay.setBrightness(0);
    }
}