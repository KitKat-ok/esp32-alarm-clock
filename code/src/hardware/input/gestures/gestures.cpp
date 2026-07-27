#include "gestures.h"

static bool gestureActivated = false;
static TaskHandle_t gestureTaskHandle = NULL;

static GestureState currentGesture = {};
static std::mutex gestureMut;
static bool gesturePending = false;
static GestureCallback onGestureDetected = nullptr;

void playGestureSound() {
    tone(BUZZER_PIN, NOTE_A4, 1000 / 24);
}

void registerGestureCallback(GestureCallback cb) {
    onGestureDetected = cb;
}

GestureState useGesture() {
    std::lock_guard<std::mutex> lock(gestureMut);

    GestureState out = currentGesture;
    currentGesture.detected = false;
    currentGesture.gestureType = DIR_NONE;
    gesturePending = false;

    return out;
}

GestureState useAllGesture() {
    std::lock_guard<std::mutex> lock(gestureMut);
    return currentGesture;
}

void setGesture(const GestureState &g) {
    std::lock_guard<std::mutex> lock(gestureMut);
    currentGesture = g;
    gesturePending = true;
}

void resumeGestureTask() {
    if (gestureTaskHandle != NULL) {
        eTaskState taskState = eTaskGetState(gestureTaskHandle);
        if (taskState == eSuspended) {
            vTaskResume(gestureTaskHandle);
        }
    }
}

void loopGestureTask(void *parameter) {
    gestureActivated = true;
    uint32_t lastEvent = 0;

    while (true) {
        uint32_t now = millis();

        if (apds.isGestureAvailable()) {
            int gesture = apds.readGesture();

            if (gesture != DIR_NONE && (now - lastEvent > GESTURE_DEBOUNCE_MS)) {
                lastEvent = now;
                playGestureSound();

                GestureState g = {};
                g.detected = true;
                g.gestureType = gesture;
                g.timestamp = now;

                inputDetected = true;
                setGesture(g);

                if (onGestureDetected != nullptr) {
                    onGestureDetected(gesture);
                }
            }
        }

        // Suspend the task until resumed by ISR or resumeGestureTask()
        vTaskSuspend(NULL);

        vTaskDelay(5 / portTICK_PERIOD_MS);
    }
}

void initGestureTask() {
    xTaskCreate(
        loopGestureTask,
        "GestureTask",
        4096,
        NULL,
        12,
        &gestureTaskHandle
    );
}
void initGestureSensor() {
    if (!apds.init()) {
        Serial.println(F("[GestureSensor] Init failed!"));
        return;
    }

    if (!apds.enableGestureSensor(true)) {
        Serial.println(F("[GestureSensor] Enable failed!"));
        return;
    }

    Serial.println(F("[GestureSensor] Initialized successfully."));
}

void turnOnGesture() {
    if (!gestureActivated) {
        initGestureSensor();
        initGestureTask();
    }
}