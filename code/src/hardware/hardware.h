#ifndef HARDWARE_H
#define HARDWARE_H

#include "../defines.h"

extern TM1637Display LedDisplay;

extern LTR_F216A lightMeter;

extern OneWire oneWire;
extern DallasTemperature tempSensor;

void initHardware();
void LowBattery();

#endif
