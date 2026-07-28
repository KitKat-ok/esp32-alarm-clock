#include "gestures.h"

static bool gestureActivated = false;
static TaskHandle_t gestureTaskHandle = NULL;

static std::mutex gestureMut;

void resumeGestureTask()
{
    if (gestureTaskHandle != NULL)
    {
        eTaskState taskState = eTaskGetState(gestureTaskHandle);
        if (taskState == eSuspended)
        {
            vTaskResume(gestureTaskHandle);
        }
    }
}

#if GESTURES_ENABLED == true

void playGestureSound()
{
    tone(BUZZER_PIN, NOTE_B4, 1000 / 10);
}

static bool gesturePending = false;
static GestureCallback onGestureDetected = nullptr;
static GestureState currentGesture = {};

void registerGestureCallback(GestureCallback cb)
{
    onGestureDetected = cb;
}

GestureState useGesture()
{
    std::lock_guard<std::mutex> lock(gestureMut);

    GestureState out = currentGesture;
    currentGesture.detected = false;
    currentGesture.gestureType = 0;
    gesturePending = false;

    return out;
}

GestureState useAllGesture()
{
    std::lock_guard<std::mutex> lock(gestureMut);
    return currentGesture;
}

void setGesture(const GestureState &g)
{
    std::lock_guard<std::mutex> lock(gestureMut);
    currentGesture = g;
    gesturePending = true;
}

void loopGestureTask(void *parameter)
{
    gestureActivated = true;

    while (true)
    {
        uint32_t now = millis();

        if (apds.gestureValid())
        {
            bool firstGestureCaptured = false;

            while (apds.gestureValid())
            {
                uint8_t gesture = apds.readGesture();

                if (gesture != 0 && !firstGestureCaptured)
                {
                    firstGestureCaptured = true;

                    Serial.println("Gesture type: " + String(gesture));

                    GestureState g = {};
                    g.detected = true;
                    g.gestureType = gesture;
                    g.timestamp = now;

                    inputDetected = true;
                    setGesture(g);
                    playGestureSound();

                    if (onGestureDetected != nullptr)
                    {
                        onGestureDetected(gesture);
                    }
                }
            }

            Wire.beginTransmission(APDS9960_ADDRESS);
            Wire.write(0xE6);
            Wire.endTransmission();

            apds.clearInterrupt();
        }

        vTaskSuspend(NULL);
        vTaskDelay(5 / portTICK_PERIOD_MS);
    }
}

#else

void loopGestureTask(void *parameter)
{
    gestureActivated = true;

    while (true)
    {
        apds.disableProximityInterrupt();

        uint8_t prox = apds.readProximity();

        Serial.println("Proximity trigger fired once! Value: " + String(prox));
        inputDetected = true;
        tone(BUZZER_PIN, NOTE_B4, 30);

        while (apds.readProximity() >= PROXIMITY_THRESHOLD)
        {
            vTaskDelay(50 / portTICK_PERIOD_MS);
        }

        apds.clearInterrupt();
        tone(BUZZER_PIN, NOTE_B3, 25);
        apds.enableProximityInterrupt();

        vTaskSuspend(NULL);
        vTaskDelay(5 / portTICK_PERIOD_MS);
    }
}

#endif // GESTURES_ENABLED

void initGestureTask()
{
    xTaskCreate(
        loopGestureTask,
        "GestureTask",
        4096,
        NULL,
        12,
        &gestureTaskHandle);
}

void turnOnGesture()
{
    if (!gestureActivated)
    {
        initGestureTask();
    }
}