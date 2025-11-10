#include "touch.h"

touchStates interruptedTouch = No_Seg;

// No logs here because interrupts don't like them
void resumeTouchTask()
{
    Serial.println("Resuming Touch task");
    eTaskState taskState = eTaskGetState(touchTask);
    if(taskState == eSuspended) {
        vTaskResume(touchTask);
    } 
#if DEBUG
    else {
        if(taskState != eRunning) {
            //Serial.println("Something is wrong with button task: " + String(taskState));
        }
    }
#endif
}

void touchInt()
{
    interruptedTouch = First_Seg;
    resumeTouchTask();
}


void turnOnTouchInterrupts()
{
    touch_sensor.attachChangeCallback(touchInt);
    Serial.println("Attached touch interrupts!");
}

void setTouchInterrupt(uint8_t pin, uint16_t threshold)
{

}