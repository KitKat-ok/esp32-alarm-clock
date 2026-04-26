#include "touch.h"

bool touchInterrupt = false;

void resumeTouchTask()
{
    eTaskState taskState = eTaskGetState(touchTask);
    if (taskState == eSuspended)
    {
        vTaskResume(touchTask);
    }
}

void touchInt()
{
    touchInterrupt = true;
    resumeTouchTask();
}

void turnOnTouchInterrupts()
{
    attachInterrupt(digitalPinToInterrupt(TOUCH_INTERRUPT), touchInt, FALLING);
    Serial.println("Attached touch interrupts!");
}
