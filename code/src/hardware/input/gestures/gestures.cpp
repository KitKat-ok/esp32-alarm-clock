#include "gestures.h"

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

    while (true)
    {
        uint32_t now = millis();

        bool isGestureValid = false;
        if (lockI2C())
        {
            isGestureValid = apds.gestureValid();
            unlockI2C();
        }

        if (isGestureValid)
        {
            bool firstGestureCaptured = false;

            while (true)
            {
                uint8_t gesture = 0;
                bool valid = false;

                if (lockI2C())
                {
                    valid = apds.gestureValid();
                    if (valid)
                    {
                        gesture = apds.readGesture();
                    }
                    unlockI2C();
                }

                if (!valid)
                {
                    break;
                }

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

            if (lockI2C())
            {
                Wire.beginTransmission(APDS9960_ADDRESS);
                Wire.write(0xE6);
                Wire.endTransmission();

                apds.clearInterrupt();
                unlockI2C();
            }
        }

        vTaskSuspend(NULL);
        vTaskDelay(5 / portTICK_PERIOD_MS);
    }
}

#else

static bool proximityPending = false;
static ProximityCallback onProximityDetected = nullptr;
static ProximityState currentProximity = {};

void registerProximityCallback(ProximityCallback cb)
{
    onProximityDetected = cb;
}

ProximityState useProximity()
{
    std::lock_guard<std::mutex> lock(gestureMut);

    ProximityState out = currentProximity;
    currentProximity.detected = false;
    currentProximity.proxValue = 0;
    proximityPending = false;

    return out;
}

ProximityState useAllProximity()
{
    std::lock_guard<std::mutex> lock(gestureMut);
    return currentProximity;
}

void setProximity(const ProximityState &p)
{
    std::lock_guard<std::mutex> lock(gestureMut);
    currentProximity = p;
    proximityPending = true;
}

void loopGestureTask(void *parameter)
{

    while (true)
    {

        uint32_t now = millis();

        uint8_t prox = 0;
        if (lockI2C())
        {
            prox = apds.readProximity();
            unlockI2C();
        }

        Serial.println("Proximity trigger fired once! Value: " + String(prox));
        inputDetected = true;

        // 1. Create and post ProximityState event
        ProximityState p = {};
        p.detected = true;
        p.proxValue = prox;
        p.timestamp = now;

        setProximity(p);
        tone(BUZZER_PIN, NOTE_B4, 30);

        // 2. Trigger optional callback
        if (onProximityDetected != nullptr)
        {
            onProximityDetected(prox);
        }

        if (lockI2C())
        {
            apds.disableProximityInterrupt();
            unlockI2C();
        }

        // 3. Block until target moves away
        while (true)
        {
            uint8_t currentProx = 0;
            if (lockI2C())
            {
                currentProx = apds.readProximity();
                unlockI2C();
            }

            if (currentProx < PROXIMITY_THRESHOLD)
            {
                break;
            }

            vTaskDelay(50 / portTICK_PERIOD_MS);
        }

        if (lockI2C())
        {
            apds.clearInterrupt();
            apds.enableProximityInterrupt();
            unlockI2C();
        }

        tone(BUZZER_PIN, NOTE_B3, 25);
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

    initGestureTask();
}