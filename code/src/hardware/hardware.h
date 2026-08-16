#pragma once

#include "../defines.h"

void initHardware();
bool readHallSwitch();

void waitForSerialInput();
void turnOnLeds(bool maxPower = false);
void turnOffLeds();
void toggleLeds(bool maxPower = false);