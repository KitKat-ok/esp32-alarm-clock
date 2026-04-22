#pragma once

#include "../defines.h"

extern AS1115 LedDisplay;

extern AT42QT2120 touch_sensor;

void initHardware();
bool readHallSwitch();

void waitForSerialInput();
