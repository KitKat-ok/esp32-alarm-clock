#pragma once

#include "defines.h"

struct touchState
{
    bool touched;
    bool held;
    bool longPress;
    bool butoonsPressed[7];
};

extern AT42QT2120 touch_sensor;
void initTouch();
void setTouchLowPower();
void setTouchNormalPower();

extern bool touchActivated;
extern std::mutex touchMut;
extern bool touchInterrupt;
extern TaskHandle_t touchTask;

touchState useTouch();
touchState useAllTouch();

void turnOnTouch();
void turnOnTouchInterrupts();