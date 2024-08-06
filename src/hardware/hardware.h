#ifndef HARDWARE_H
#define HARDWARE_H

#include "../defines.h"

extern OLED_SSD1306_Chart display;

extern TM1637Display LedDisplay;

extern BH1750 lightMeter;

extern OneWire oneWire;
extern DallasTemperature tempSensor;

void initHardware();
void LowBattery();

#endif
