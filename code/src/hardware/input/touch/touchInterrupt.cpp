#include "touch.h"

touchStates interruptedTouch = No_Seg;

// No logs here because interrupts don't like them
void resumeTouchTask()
{
    //Serial.println("Resuming button task");
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

void firstSegInt()
{
    interruptedTouch = First_Seg;
    resumeTouchTask();
}

void secondSegInt()
{
    interruptedTouch = Second_Seg;
    resumeTouchTask();
}

void thirdSegInt()
{
    interruptedTouch = Third_Seg;
    resumeTouchTask();
}

void fourthSegInt()
{
    interruptedTouch = Fourt_Seg;
    resumeTouchTask();
}

void fifthSegInt()
{
    interruptedTouch = Fifth_Seg;
    resumeTouchTask();
}

void turnOnTouchInterrupts()
{
    Serial.println("Attached touch interrupts!");
}

void setTouchInterrupt(uint8_t pin, uint16_t threshold)
{

}