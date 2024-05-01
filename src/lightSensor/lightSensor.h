#ifndef LIGHTSENSOR_H
#define LIGHTSENSOR_H

#include "../defines.h"

void createDimmingTask();

void dimLedDisplay();

float smoothLightReading();

extern TaskHandle_t dimmingTask;
extern bool dimmingTaskRunning;
extern bool maxBrightness;
extern bool displayON;


#endif
