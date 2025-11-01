#ifndef HARDWARE_H
#define HARDWARE_H

#include "../defines.h"

extern AS1115 LedDisplay;

extern LTR_F216A lightMeter;

extern Adafruit_SHT4x sht4;

void initHardware();
bool readHallSwitch();

#endif
