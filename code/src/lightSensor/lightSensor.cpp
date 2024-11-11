#include "defines.h"

void dimmingFunction(void *pvParameters);
void dimOledDisplay();
void dimLedDisplay();

bool maxBrightness = false;

bool dimmingTaskRunning = false;

bool displayON = true;

bool inputDetected = false;

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

float lightLevel = 0.0;

void dimmingFunction(void *pvParameters)
{
    const unsigned long intervalLight = 60000;
    unsigned long previousMillisLight = 0;

    unsigned long previousMillisDimming = 0;
    unsigned long intervalDimming = 1000;

    unsigned long lastActionTime = 0;
    unsigned long delayDuration = 60000;
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
                lightLevel = removeLightNoise();
                dimOledDisplay();
                maxBrightness = false;

                Serial.println("touch level: " + String(touchRead(TOUCH_BUTTON_PIN)));

                Serial.println("Reading brightness and dimming oled accordingly");
                previousMillisDimming = currentMillis;
            }
            vTaskDelay(10);
            if (checkForInput() == true)
            {
                inputDetected = true;
                manager.oledEnable();
                delay(50);
                break;
            }
        }
        Serial.println("Button pressed");
        Serial.println("touch level: " + String(touchRead(TOUCH_BUTTON_PIN)));

        Serial.println("setting max brightness");
        vTaskDelay(pdMS_TO_TICKS(20));

        maxBrightness = true;
        lastActionTime = millis();
        vTaskDelay(pdMS_TO_TICKS(350));
        if (dimmed == true)
        {
            manager.oledFadeIn();
            dimmed = false;
        }

        vTaskDelay(pdMS_TO_TICKS(10));
        while (millis() - lastActionTime < delayDuration)
        {
            lightLevel = removeLightNoise();

            vTaskDelay(10);
            if (checkForInput() == true)
            {
                lastActionTime = millis();
                if (menuRunning == false)
                {
                    turnOffScreensaver();
                }
                if (dimmed == true)
                {
                    manager.oledFadeIn();
                    dimmed = false;
                }

                vTaskDelay(10);
                while (checkForInput() == true)
                {
                    lastActionTime = millis();
                }
            }
        }
        inputDetected = false;
    }
}

float previousLightLevel = 0.0;

void dimOledDisplay()
{
    int currentHour = hour();
    int currentMinute = minute();
    Serial.println("raw light level: " + String(removeLightNoise()));
    Serial.println("smoothened light level: " + String(lightLevel));

    if (OLED_DISABLE_THRESHOLD > lightLevel)
    {
        manager.oledDisable();

        if (dimmed == false)
        {
            manager.oledFadeOut();
            dimmed = true;
            delay(50);
        }
        delay(50);
    }
    else
    {
        manager.oledEnable();

        if (dimmed == false)
        {
            manager.oledFadeOut();
            dimmed = true;
        }
    }

    Serial.println("display off");
}

void dimLedDisplay()
{
    int currentHour = hour();
    int currentMinute = minute();
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

const int bufferLightSize = 30; // Adjust buffer size as needed
float lightLevelBuffer[bufferLightSize];
int bufferLightIndex = 0;

float smoothedLightLevel = 0.0; // Initial smoothed light level

// New parameters for adjustment
float normalSmoothingFactor = 0.005;  // Normal smoothing factor (adjust as needed)
float highChangeSmoothingFactor = 0.5;  // Smoothing factor for significant changes (adjust as needed)
float significantChangeThreshold = 5.0; // Threshold to classify a big change (adjust as needed)
float spikeSuppressionThreshold = MAX_INCREASE_OF_LIGHT_LEVEL; // Suppress spikes beyond this value

float removeLightNoise()
{
    float currentLightLevel = lightMeter.readLightLevel(); // Read the current light level from BH1750 sensor

    // Handle sudden spikes in light level by suppressing them
    if (currentLightLevel - lightLevelBuffer[bufferLightIndex] > spikeSuppressionThreshold)
    {
        currentLightLevel = lightLevelBuffer[bufferLightIndex]; // Suppress the spike by keeping the last good value
    }

    // Store current light level in the circular buffer
    lightLevelBuffer[bufferLightIndex] = currentLightLevel;

    // Update buffer index (circular increment)
    bufferLightIndex = (bufferLightIndex + 1) % bufferLightSize;

    // Calculate simple moving average
    float movingAverage = 0.0;
    for (int i = 0; i < bufferLightSize; ++i)
    {
        movingAverage += lightLevelBuffer[i];
    }
    movingAverage /= bufferLightSize;

    // Determine if this is a significant change
    float lightLevelDifference = abs(currentLightLevel - smoothedLightLevel);
    if (lightLevelDifference > significantChangeThreshold)
    {
        // Use the adjustable smoothing factor for significant changes
        smoothedLightLevel = highChangeSmoothingFactor * movingAverage + (1.0 - highChangeSmoothingFactor) * smoothedLightLevel;
    }
    else
    {
        // Use the normal smoothing factor for small changes
        smoothedLightLevel = normalSmoothingFactor * movingAverage + (1.0 - normalSmoothingFactor) * smoothedLightLevel;
    }

    return smoothedLightLevel;
}




int touchSamples[NUM_TOUCH_SAMPLES];
int touchSampleIndex = 0;

int smoothTouchRead(int pin)
{
    int sum = 0;
    touchSamples[touchSampleIndex] = touchRead(pin);
    touchSampleIndex = (touchSampleIndex + 1) % NUM_TOUCH_SAMPLES;

    for (int i = 0; i < NUM_TOUCH_SAMPLES; i++)
    {
        sum += touchSamples[i];
    }

    return sum / NUM_TOUCH_SAMPLES;
}

bool checkForInput()
{
    int threshold;
    if (charging == false)
    {
        threshold = TOUCH_BUTTON_THRESHOLD_ON_BATTERY;
    }
    else if (inputDetected == true)
    {
        threshold = TOUCH_BUTTON_THRESHOLD_WHEN_ALREADY_TOUCHED;
    }
    else
    {
        threshold = TOUCH_BUTTON_THRESHOLD;
    }

    if (smoothTouchRead(TOUCH_BUTTON_PIN) < threshold ||
        digitalRead(BUTTON_UP_PIN) == LOW ||
        digitalRead(BUTTON_DOWN_PIN) == LOW ||
        digitalRead(BUTTON_CONFIRM_PIN) == LOW ||
        digitalRead(BUTTON_EXIT_PIN) == LOW)
    {
        delay(50);
        if (smoothTouchRead(TOUCH_BUTTON_PIN) < threshold ||
            digitalRead(BUTTON_UP_PIN) == LOW ||
            digitalRead(BUTTON_DOWN_PIN) == LOW ||
            digitalRead(BUTTON_CONFIRM_PIN) == LOW ||
            digitalRead(BUTTON_EXIT_PIN) == LOW)
        {
            return true;
        }
        else
        {
            return true;
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

    if ((alarms[weekdayIndex].hours == 0 && alarms[weekdayIndex].minutes == 0) && alarms[weekdayIndex].enabled == false)
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