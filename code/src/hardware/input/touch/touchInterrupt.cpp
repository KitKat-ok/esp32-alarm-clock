#include "touch.h"

touchStates interruptedTouch = No_Seg;

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
    resumeTouchTask();
}

void turnOnTouchInterrupts()
{
    attachInterrupt(digitalPinToInterrupt(TOUCH_INTERRUPT), touchInt, FALLING);
    Serial.println("Attached touch interrupts!");
}

void setTouchInterrupt(uint8_t pin, uint16_t threshold)
{
}