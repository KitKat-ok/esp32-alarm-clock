#include "defines.h"

#define AL_ADDR 0x10
SparkFun_Ambient_Light lightMeter(AL_ADDR);

void dimmingTask(void *pvParameters);
void oledWakeupTask(void *pvParameters);
void dimOledDisplay();
void dimLedDisplay();

bool maxBrightness = false;

bool dimmingTaskRunning = false;

bool LedDisplayOn = true;

bool inputDetected = false;

TaskHandle_t dimmingTaskHandle;
TaskHandle_t oledWakeupTaskHandle;

void lightTask(void *pvParameters)
{
    unsigned long previousMillisChart = 0;
    while (true)
    {
        unsigned long currentMillis = millis(); // Get the current time
        if (currentMillis - previousMillisChart >= INTERVAL_CHARTS)
        {
            lightLevel = getLightLevel();
            for (int i = 0; i < CHART_READINGS - 1; i++)
            {
                lightArray[i] = lightArray[i + 1];
            }
            lightArray[CHART_READINGS - 1] = lightLevel;
            previousMillisChart = currentMillis;
        }
        vTaskDelay(pdMS_TO_TICKS(600000));
    }
}

void createLightTask()
{
    xTaskCreate(
        lightTask,   /* Task function. */
        "lightTask", /* String with name of task. */
        2048,        /* Stack size in words. */
        NULL,        /* Parameter passed as input of the task */
        1,           /* Priority of the task. */
        NULL         /* Task handle. */
    );
}

void createDimmingTask()
{
    Serial.print("creating dimmingTask");
    xTaskCreate(
        dimmingTask,       /* Task function. */
        "DimTask",         /* String with name of task. */
        4096,              /* Stack size in words. */
        NULL,              /* Parameter passed as input of the task */
        1,                 /* Priority of the task. */
        &dimmingTaskHandle /* Task handle. */
    );

    xTaskCreate(
        oledWakeupTask,       /* Task function. */
        "InputOledTask",      /* String with name of task. */
        4096,                 /* Stack size in words. */
        NULL,                 /* Parameter passed as input of the task */
        3,                    /* Priority of the task. */
        &oledWakeupTaskHandle /* Task handle. */
    );
}

float lightLevel = 0.0;
int mmwaveState = 0;

void oledWakeupTask(void *pvParameters)
{
    unsigned long lastActionTime = 0;
    while (true)
    {
        if (useAllButtons() != None || useAllTouch().touched == true || inputDetected == true)
        {
            vTaskSuspend(dimmingTaskHandle);
            vTaskResume(TimeTask);
            Serial.println("Button pressed");
            Serial.println("Setting max brightness");

            inputDetected = true;
            maxBrightness = true;
            lastActionTime = millis();

            oledMana.enable();
            Serial.print("lock mutex");
            showCurrentTime();
            LedMut.lock();

            if (currentWeatherData.isDay == false)
            {
                setLedIntensity(LED_BRIGHTNESS_MAX_NIGHT);
            }
            else
            {
                setLedIntensity(LED_BRIGHTNESS_MAX);
            }
            LedMut.unlock();
            Serial.print("unlock mutex");

            if (oledMana.dimmed)
            {
                oledMana.fadeIn();
                delay(5);
            }
            inputDetected = false;

            vTaskDelay(pdMS_TO_TICKS(100));

            while (millis() - lastActionTime < DIM_DELAY)
            {
                vTaskDelay(pdMS_TO_TICKS(5));

                if (useAllButtons() != None || useAllTouch().touched == true || inputDetected == true)
                {
                    inputDetected = false;
                    lastActionTime = millis();

                    if (!oledMana.ScreenEnabled)
                    {
                        oledMana.enable();
                    }

                    vTaskDelay(pdMS_TO_TICKS(5));

                    if (useAllButtons() != None || useAllTouch().touched == true || inputDetected == true)
                    {
                        inputDetected = false;
                        lastActionTime = millis();
                    }
                }
            }
            if (WiFi.isConnected() && WiFi.SSID() == SSID1)
            {
                mmwaveState = getMmwaveState();
            }
            lightLevel = getLightLevel();
            dimOledDisplay();
            dimLedDisplay();
        }
        else
        {
            eTaskState dimmingTaskState = eTaskGetState(dimmingTaskHandle);
            if (dimmingTaskState == eSuspended)
            {
                vTaskResume(dimmingTaskHandle);
            }
            delay(10);
        }
    }
}

void dimmingTask(void *pvParameters)
{
    unsigned long previousMillisDimming = 0;
    unsigned long intervalDimming = 1000;

    unsigned long previousMillisLight = 0;

    unsigned long previousMillisState = 0;
    unsigned long intervalState = 30000;

    while (true)
    {
        dimmingTaskRunning = true;
        unsigned long currentMillis = millis();

        if (currentMillis - previousMillisLight >= intervalDimming)
        {
            lightLevel = getLightLevel();
            previousMillisLight = currentMillis;
        }

        if (currentMillis - previousMillisState >= intervalState && WiFi.isConnected() && WiFi.SSID() == SSID1)
        {
            mmwaveState = getMmwaveState();
            previousMillisState = currentMillis;
            delay(100);
        }

        if (currentMillis - previousMillisDimming >= intervalDimming)
        {
            dimLedDisplay();
            dimOledDisplay();
            maxBrightness = false;

            Serial.println("dimming OLED accordingly: " + String(lightLevel));
            previousMillisDimming = currentMillis;
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

bool shouldTurnOffDisplay(int lux)
{
    static const int offThreshold = OLED_DISABLE_THRESHOLD;     // Lux threshold to turn off
    static const int onThreshold = OLED_DISABLE_THRESHOLD + 10; // Lux threshold to turn back on
    static bool isDisplayOff = false;

    if (isDisplayOff && lux >= onThreshold)
    {
        isDisplayOff = false; // Turn on when lux exceeds onThreshold
    }
    else if (!isDisplayOff && lux < offThreshold)
    {
        isDisplayOff = true; // Turn off when lux drops below offThreshold
    }

    return isDisplayOff;
}

float previousLightLevel = 0.0;

void dimOledDisplay()
{
    if (shouldTurnOffDisplay(lightLevel) == true || (mmwaveState == 0 && WiFi.SSID() == SSID1 && mmwaveState != 3 && WiFi.isConnected() == true))
    {
        oledMana.disable();

        if (oledMana.dimmed == false)
        {
            oledMana.fadeOut();
            delay(50);
        }
        delay(50);
    }
    else
    {
        oledMana.enable();

        if (oledMana.dimmed == false)
        {
            oledMana.fadeOut();
        }
    }
}

static int ledLastBrightness = LED_BRIGHTNESS_MIN;

int mapWithHysteresis(uint8_t lightLevel)
{
    uint8_t maxBright = LED_BRIGHTNESS_MAX_NIGHT;
    if (currentWeatherData.isDay == false)
    {
        maxBright = LED_BRIGHTNESS_MAX_NIGHT;
    }
    else
    {
        maxBright = LED_BRIGHTNESS_MAX;
    }

    uint8_t newBrightness = (lightLevel - LED_DIM_THRESHOLD) * (maxBright - LED_BRIGHTNESS_MIN) /
                                (LED_MAP_MAX_LIGHT - LED_DIM_THRESHOLD) +
                            LED_BRIGHTNESS_MIN;

    newBrightness = constrain(newBrightness, LED_BRIGHTNESS_MIN, maxBright);

    if (newBrightness > ledLastBrightness + LED_HYSTERESIS)
    {
        ledLastBrightness = newBrightness;
    }
    else if (newBrightness < ledLastBrightness - LED_HYSTERESIS)
    {
        ledLastBrightness = newBrightness;
    }

    return ledLastBrightness;
}

bool disableHysteresisState = false;

void dimLedDisplay()
{
    if (lightLevel < 5000)
    {
        if (disableHysteresisState)
        {
            if (lightLevel > LED_DISABLE_THRESHOLD + 3) // hysteresis upper limit
                disableHysteresisState = false;
        }
        else
        {
            if (lightLevel <= LED_DISABLE_THRESHOLD - 3 && checkForNight())
                disableHysteresisState = true;
        }
        LedMut.lock();
        if (disableHysteresisState)
        {
            LedDisplay.clear();
            LedDisplayOn = false;
        }
        else if (lightLevel > LED_DIM_THRESHOLD)
        {
            LedDisplayOn = true;
            uint8_t brightness = mapWithHysteresis(lightLevel);
            setLedIntensity(brightness);
            Serial.println("Brightness of Led display " + String(brightness));
        }
        else
        {
            LedDisplayOn = true;
            setLedIntensity(0);
            Serial.println("Brightness of Led display 0");
        }
        LedMut.unlock();
    }
}

bool checkForNight()
{
    if (isWeatherAvailable == false)
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
    else
    {
        return !currentWeatherData.isDay;
    }
}

int getMmwaveState()
{
    String url = String(LIGHT_IP) + "mmwave";
    String jsonString;
    const int maxRetries = 3;

    for (int attempt = 0; attempt < maxRetries; ++attempt)
    {
        jsonString = getStringRequest(url);

        if (jsonString.length() > 0)
        {
            break; 
        }
        else
        {
            Serial.println("Failed to fetch data, retrying...");
            delay(1000);
        }
    }

    if (jsonString.length() == 0)
    {
        Serial.println("Failed to fetch data after multiple attempts.");
        return 3;
    }

    JsonDocument jsonDoc;
    DeserializationError error = deserializeJson(jsonDoc, jsonString);

    if (error)
    {
        Serial.print("deserializeJson() returned ");
        Serial.println(error.c_str());
        return 3; // Return error code if deserialization fails
    }

    bool state = jsonDoc["Detected"];

    if (state)
    {
        Serial.println("Detected mmWave");
        return 1;
    }

    Serial.println("Not Detected mmWave");
    return 0;
}

float getLightLevel()
{
    float currentLightLevel = lightMeter.readLight(); // Read the current light level from BH1750 sensor
    return currentLightLevel;
}

void initLightSensor()
{
  // Possible values: .125, .25, 1, 2
  // Both .125 and .25 should be used in most cases except darker rooms.
  // A gain of 2 should only be used if the sensor will be covered by a dark
  // glass.
  float gain = 1;

  // Possible integration times in milliseconds: 800, 400, 200, 100, 50, 25
  // Higher times give higher resolutions and should be used in darker light.
  int time = 400;

  if (lightMeter.begin(Wire))
    Serial.println("Ready to sense some light!");
  else
    Serial.println("Could not communicate with the sensor!");

  // Again the gain and integration times determine the resolution of the lux
  // value, and give different ranges of possible light readings. Check out
  // hoookup guide for more info.
  lightMeter.setGain(gain);
  lightMeter.setIntegTime(time);

  Serial.println("Reading settings...");
  Serial.print("Gain: ");
  float gainVal = lightMeter.readGain();
  Serial.print(gainVal, 3);
  Serial.print(" Integration Time: ");
  int timeVal = lightMeter.readIntegTime();
  Serial.println(timeVal);
}
