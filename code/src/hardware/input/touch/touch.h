#pragma once

#include "defines.h"

struct touchState
{
    bool touched;
    bool held;
    bool longPress;
    bool butoonsPressed[7];
};

extern bool touchActivated;
extern std::mutex touchMut;
extern bool touchInterrupt;
extern TaskHandle_t touchTask;

touchState useTouch();
touchState useAllTouch();

void setTouchInterrupt(uint8_t pin, uint16_t threshold);
void turnOnTouchInterrupts();

void turnOnTouch();