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
        1                /* Core where the task should run. */
    );
}

bool dimmed = false;
bool fading = false;

void oledFadeout()
{
    if (fading == false)
    {
        fading = true;
        display.ssd1306_command(SSD1306_DISPLAYON);
        for (int dim = 150; dim >= 0; dim -= 10)
        {
            display.ssd1306_command(0x81);
            display.ssd1306_command(dim); // max 157
            delay(10);
        }

        for (int dim2 = 34; dim2 >= 0; dim2 -= 17)
        {
            display.ssd1306_command(0xD9);
            display.ssd1306_command(dim2); // max 34
            delay(20);
        }
        delay(20);
        fading = false;
    }
}

void oledFadein()
{
    if (fading == false)
    {
        fading = true;
        for (int dim = 0; dim <= 160; dim += 10)
        {
            display.ssd1306_command(0x81);
            display.ssd1306_command(dim); // max 160
            delay(10);
        }

        for (int dim2 = 0; dim2 <= 34; dim2 += 17)
        {
            display.ssd1306_command(0xD9);
            display.ssd1306_command(dim2); // max 34
            delay(20);
        }
        delay(20);
        fading = false;
    }
}

void dimmingFunction(void *pvParameters)
{
    const unsigned long intervalLight = 60000;
    unsigned long previousMillisLight = 0;

    unsigned long previousMillisDimming = 0;
    unsigned long intervalDimming = 1000;

    unsigned long lastActionTime = 0;
    unsigned long delayDuration = 20000;
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
                lightArray[TEMP_CHART_READINGS - 1] = removeLightNoise(); // Replace with your temperature reading function
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
            if (checkForInput(TOUCH_BUTTON_THRESHOLD) == true)
            {
                break;
            }
        }
        Serial.println("Button pressed");
        Serial.println("touch level: " + String(touchRead(TOUCH_BUTTON_PIN)));

        Serial.println("setting max brightness");
        display.ssd1306_command(SSD1306_DISPLAYON);
        if (dimmed == true && fading == false)
        {
            display.startWrite();
            oledFadein();
            display.endWrite();
            dimmed = false;
        }
        turnOffScreensaver();
        maxBrightness = true;
        lastActionTime = millis();

        while (millis() - lastActionTime < delayDuration)
        {
            vTaskDelay(10);
            if (checkForInput(TOUCH_BUTTON_THRESHOLD) == true)
            {
                lastActionTime = millis();
            }
        }

        if (dimmed == false && fading == false)
        {
            display.startWrite();
            oledFadeout();
            display.endWrite();
            dimmed = true;
        }
    }
}

float previousLightLevel = 0.0;

void dimOledDisplay()
{
    int currentHour = hour();
    int currentMinute = minute();
    int lightLevel = removeLightNoise();
    Serial.println("raw light level: " + String(removeLightNoise()));
    Serial.println("smoothened light level: " + String(lightLevel));
    if (lightLevel < 5000)
    {
        if (lightLevel <= OLED_DISABLE_THRESHOLD)
        {
            display.ssd1306_command(SSD1306_DISPLAYOFF);
            Serial.println("display off");
        }
        else
        {
            if (dimmed == false && fading == false)
            {
                display.startWrite();
                oledFadeout();
                display.endWrite();
            }
            display.ssd1306_command(SSD1306_DISPLAYON);
            dimmed = true;
        }
    }
}

void dimLedDisplay()
{
    int currentHour = hour();
    int currentMinute = minute();
    int lightLevel = removeLightNoise();
    Serial.println("raw light level: " + String(removeLightNoise()));
    Serial.println("smoothened light level: " + String(lightLevel));
    if (lightLevel < 5000)
    {

        if (lightLevel <= LED_DISABLE_THRESHOLD && (checkForNight() == true))
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

float removeLightNoise()
{
    delay(100);
    static float smoothedLightLevel = 0.0;                 // Initial smoothed light level
    float currentLightLevel = lightMeter.readLightLevel(); // Read the current light level from BH1750 sensor

    float diff = currentLightLevel - lastLightLevel;

    // Adjust the smoothing factor based on the difference
    if (diff > MAX_INCREASE_OF_LIGHT_LEVEL)
    {
        currentLightLevel = 0.0;
    }

    lastLightLevel = lightMeter.readLightLevel();

    return currentLightLevel;
}

bool checkForInput(int thresold)
{
    if (touchRead(TOUCH_BUTTON_PIN) < thresold ||
        digitalRead(BUTTON_UP_PIN) == LOW ||
        digitalRead(BUTTON_DOWN_PIN) == LOW ||
        digitalRead(BUTTON_CONFIRM_PIN) == LOW ||
        digitalRead(BUTTON_EXIT_PIN) == LOW)
    {
        delay(50);
        if (touchRead(TOUCH_BUTTON_PIN) < thresold ||
            digitalRead(BUTTON_UP_PIN) == LOW ||
            digitalRead(BUTTON_DOWN_PIN) == LOW ||
            digitalRead(BUTTON_CONFIRM_PIN) == LOW ||
            digitalRead(BUTTON_EXIT_PIN) == LOW)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

bool checkForNight()
{
    time_t currentTime = now();
    int weekdayIndex = weekday(currentTime) - 1;

    int currentHour = hour();

    if ((alarms[weekdayIndex].hours == 0 && alarms[weekdayIndex].minutes == 0) && alarms[weekdayIndex].isSet == false)
    {
        if (currentHour >= 23 || currentHour < 10)
        {
            return true;
        }
        else
            return false;
    }
    else
    {
        if (currentHour >= 23 || currentHour < alarms[weekdayIndex].hours)
        {
            return true;
        }
        else
            return false;
    }
}