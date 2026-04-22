#include "touch.h"

#define TOUCH_DEBOUNCE 100 // Time how long touch has to be detected to be accepted recognized as a proper touch instead of noise
#define DEBOUNCE_POLL_SPEED 50

bool touchActivated = false;
TaskHandle_t touchTask = NULL;
touchState currentTouch;
std::mutex touchMut;

touchState useTouch()
{
    touchState touchCopy = currentTouch;

    currentTouch.longPress = false;
    currentTouch.touched = false;

    return touchCopy;
}

touchState useAllTouch()
{
    return currentTouch;
}

void setTouch(const touchState &touch, bool onlyHeld = false)
{
    if (onlyHeld)
    {
        currentTouch.held = touch.held;
        return;
    }

    currentTouch = touch;
}

void loopTouchTask(void *parameter)
{
    touchActivated = true;

    const int LONG_PRESS_MS = 600;

    static bool wasPressed = false;
    static uint32_t touchStartTime = 0;

    static bool lastTouched = false;
    static bool lastHeld = false;
    static bool lastLongPress = false;
    static int lastSlider = -1;

    while (true)
    {
        touchState s = {};
        s.sliderState = -1;

        AT42QT2120::Status status = touch_sensor.getStatus();
        bool pressed = status.any_key_touched;
        int slider = status.slider_or_wheel_position;
        uint32_t now = millis();

        if (pressed)
        {
            if (!wasPressed)
            {
                touchStartTime = now;
                tone(BUZZER_PIN, NOTE_C7, 1000 / 16);
            }
            else
            {
                s.held = true;
            }

            s.touched = true;
            s.sliderState = slider;

            if (now - touchStartTime >= LONG_PRESS_MS)
                s.longPress = true;

            wasPressed = true;
        }
        else
        {
            wasPressed = false;
            touchStartTime = 0;
        }

        bool heldChanged = (s.held != lastHeld);

        bool fullChange =
            s.touched != lastTouched ||
            s.sliderState != lastSlider ||
            s.longPress != lastLongPress;

        if (fullChange)
        {
            if (s.touched)
            {
                setTouch(s, false);
            }

            lastTouched = s.touched;
            lastLongPress = s.longPress;
            lastSlider = s.sliderState;
        }

        if (heldChanged)
        {
            setTouch(s, true);
            lastHeld = s.held;
        }

        touchInterrupt = false;

        if (!pressed && digitalRead(TOUCH_INTERRUPT) == HIGH)
        {
            Serial.println("Touch task Going sleep");
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
        4096, // Too much but handling fs logging takes a bit more
        NULL,
        12,
        &touchTask);
}

void turnOnTouch()
{
    if (touchActivated == false)
    {
        initTouchTask();
        turnOnTouchInterrupts();
    }
}