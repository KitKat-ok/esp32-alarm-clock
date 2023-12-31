#ifndef HARDWARE_H
#define HARDWARE_H

#include "../defines.h"

extern Adafruit_SSD1306 display;
extern TM1637Display LedDisplay;
extern BH1750 lightMeter;

void initHardware();

#endif
