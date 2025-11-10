#ifndef HARDWARE_H
#define HARDWARE_H

#include "../defines.h"

extern AS1115 LedDisplay;

extern SparkFun_Ambient_Light lightMeter;

extern Adafruit_SHT4x sht4;

extern AT42QT2120 touch_sensor;

void initHardware();
bool readHallSwitch();

#endif
