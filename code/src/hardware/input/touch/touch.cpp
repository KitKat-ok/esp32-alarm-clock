#include "touch.h"

#define TOUCH_DEBOUNCE 50
#define LONG_PRESS_MS 600

bool touchActivated = false;
TaskHandle_t touchTask = NULL;

touchState currentTouch = {};
std::mutex touchMut;

bool touchPending = false;

void playTouchSound()
{
    tone(BUZZER_PIN, NOTE_A4, 1000 / 24);
}

touchState useTouch()
{
    std::lock_guard<std::mutex> lock(touchMut);

    touchState out = currentTouch;

    currentTouch.touched = false;
    currentTouch.longPress = false;
    currentTouch.sliderState = -1;

    touchPending = false;

    return out;
}

touchState useAllTouch()
{
    std::lock_guard<std::mutex> lock(touchMut);
    return currentTouch;
}

void setTouch(const touchState &t)
{
    std::lock_guard<std::mutex> lock(touchMut);
    currentTouch = t;
    touchPending = true;
}

void loopTouchTask(void *parameter)
{
    touchActivated = true;

    bool wasPressed = false;
    bool soundPlayed = false;

    uint32_t pressStart = 0;
    uint32_t lastEvent = 0;

    while (true)
    {
        auto status = touch_sensor.getStatus();
        bool pressed = status.any_key_touched;
        int slider = status.slider_or_wheel_position;
        uint32_t now = millis();

        if (pressed)
        {
            if (!wasPressed && (now - lastEvent > TOUCH_DEBOUNCE))
            {
                pressStart = now;
                lastEvent = now;
                soundPlayed = false;
            }

            if (!soundPlayed)
            {
                playTouchSound();
                soundPlayed = true;
            }

            touchState t = {};
            t.touched = true;
            inputDetected = true;
            t.sliderState = slider;
            t.longPress = (now - pressStart >= LONG_PRESS_MS);

            setTouch(t);

            wasPressed = true;
        }
        else
        {
            wasPressed = false;
            soundPlayed = false;
        }

        if (!pressed)
        {
            vTaskSuspend(NULL);
        }

        vTaskDelay(5 / portTICK_PERIOD_MS);
    }
}

void initTouchTask()
{
    xTaskCreate(
        loopTouchTask,
        "touchTask",
        4096,
        NULL,
        12,
        &touchTask);
}

void turnOnTouch()
{
    if (!touchActivated)
    {
        initTouchTask();
        turnOnTouchInterrupts();
    }
}