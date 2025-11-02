#include "buttons.h"
#include "rtcMem/rtcMem.h"

inkButtonStates interruptedButton = None;

// No logs here because interrupts don't like them
void resumeButtonTask()
{
    //Serial.println("Resuming button task");
    eTaskState taskState = eTaskGetState(buttonTask);
    if(taskState == eSuspended) {
        vTaskResume(buttonTask);
    } 
#if DEBUG
    else {
        if(taskState != eRunning) {
            //Serial.println("Something is wrong with button task: " + String(taskState));
        }
    }
#endif
}

void backIntBut()
{
    interruptedButton = Back;
    resumeButtonTask();
}

void menuIntBut()
{
    interruptedButton = Menu;
    resumeButtonTask();
}

void upIntBut()
{
    interruptedButton = Up;
    resumeButtonTask();
}

void downIntBut()
{
    interruptedButton = Down;
    resumeButtonTask();
}

void turnOnInterrupts()
{
    rM.gpioExpander.setDefaultInterruptsEsp();
}
