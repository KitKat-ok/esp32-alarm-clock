#include "defines.h"

void dimmingFunction(void *pvParameters);
void dimOledDisplay();
void dimLedDisplay();

bool maxBrightness = false;

bool dimmingTaskRunning = false;

bool displayON = true;

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

unsigned long previousMillisDimming = 0;
unsigned long intervalDimming = 10000; // Example interval in milliseconds

unsigned long lastActionTime = 0;
unsigned long delayDuration = 15000; // 30 seconds in milliseconds

void dimmingFunction(void *pvParameters)
{
    while (true)
    {
        dimmingTaskRunning = true;

        int currentHour = hour();
        int currentMinute = minute();

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

            if (currentMillis - previousMillisDimming >= intervalDimming && charging == true)
            {
                dimOledDisplay();
                maxBrightness = false;

                Serial.println("touch level: " + String(touchRead(TOUCH_BUTTON_PIN)));

                Serial.println("Reading brightness and dimming oled accordingly");
                previousMillisDimming = currentMillis;
            }
            vTaskDelay(10);
            if (touchRead(TOUCH_BUTTON_PIN) < TOUCH_BUTTON_THRESHOLD ||
                digitalRead(BUTTON_UP_PIN) == LOW ||
                digitalRead(BUTTON_DOWN_PIN) == LOW ||
                digitalRead(BUTTON_CONFIRM_PIN) == LOW ||
                digitalRead(BUTTON_EXIT_PIN) == LOW)
            {
                break;
            }
        }

        Serial.println("Button pressed");
        Serial.println("touch level: " + String(touchRead(TOUCH_BUTTON_PIN)));

        Serial.println("setting max brightness");
        display.ssd1306_command(SSD1306_DISPLAYON);
        display.dim(false);
        maxBrightness = true;
        lastActionTime = millis();

        while (touchRead(TOUCH_BUTTON_PIN) < TOUCH_BUTTON_THRESHOLD ||
               digitalRead(BUTTON_UP_PIN) == LOW ||
               digitalRead(BUTTON_DOWN_PIN) == LOW ||
               digitalRead(BUTTON_CONFIRM_PIN) == LOW ||
               digitalRead(BUTTON_EXIT_PIN) == LOW)
        {
            lastActionTime = millis();
            vTaskDelay(10);

            while (millis() - lastActionTime < delayDuration)
            {
                vTaskDelay(10);
                if (touchRead(TOUCH_BUTTON_PIN) < TOUCH_BUTTON_THRESHOLD ||
                    digitalRead(BUTTON_UP_PIN) == LOW ||
                    digitalRead(BUTTON_DOWN_PIN) == LOW ||
                    digitalRead(BUTTON_CONFIRM_PIN) == LOW ||
                    digitalRead(BUTTON_EXIT_PIN) == LOW)
                {
                    lastActionTime = millis();
                }
            }
        }
    }
}

float previousLightLevel = 0.0;

void dimOledDisplay()
{
    int currentHour = hour();
    int currentMinute = minute();
    int lightLevel = smoothLightReading();
    Serial.println("raw light level: " + String(lightMeter.readLightLevel()));
    Serial.println("smoothened light level: " + String(lightLevel));
    if (lightLevel < 5000)
    {
        if (lightLevel <= 1.0 && (currentHour >= 23 || currentHour < 10))
        {
            display.ssd1306_command(SSD1306_DISPLAYOFF);
            Serial.println("display off");
        }
        else if (lightLevel > OLED_DIM_THRESHOLD)
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
}

void dimLedDisplay()
{
    int currentHour = hour();
    int currentMinute = minute();
    int lightLevel = smoothLightReading();
    Serial.println("raw light level: " + String(lightMeter.readLightLevel()));
    Serial.println("smoothened light level: " + String(lightLevel));
    if (lightLevel < 5000)
    {

        if (lightLevel <= 1.00 && (currentHour >= 23 || currentHour < 10))
        {
            LedDisplay.clear();
            displayON = false;
        }
        else if (lightLevel > LED_DIM_THRESHOLD)
        {
            displayON = true;
            LedDisplay.setBrightness(map(constrain(lightLevel, LED_DIM_THRESHOLD, 100), LED_DIM_THRESHOLD, 100, 0, 7));
            Serial.println("Brightness of Led display " + String(map(constrain(lightLevel, 0, LED_DIM_THRESHOLD), 0, LED_DIM_THRESHOLD, 0, 7)));
        }
        else
        {
            displayON = true;
            LedDisplay.setBrightness(0);
        }
    }
}

float lastLightLevel = 0.0;

float smoothLightReading() {
    delay(100);
    static float smoothedLightLevel = 0.0; // Initial smoothed light level
    float currentLightLevel = lightMeter.readLightLevel(); // Read the current light level from BH1750 sensor

    float diff = currentLightLevel - lastLightLevel;

    // Adjust the smoothing factor based on the difference
    float smoothingFactor = LIGHT_SMOOTHING_FACTOR;
    if (diff > MAX_INCREASE_OF_LIGHT_LEVEL) {
        smoothingFactor = 1.0; // Use full weight if the increase exceeds the threshold
        currentLightLevel = 0.0;
    }

    lastLightLevel = lightMeter.readLightLevel();

    return currentLightLevel;
}