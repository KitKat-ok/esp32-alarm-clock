#ifndef LIGHTSENSOR_H
#define LIGHTSENSOR_H

#include "../defines.h"

void createDimmingTask();

void dimLedDisplay();

float removeLightNoise();
int smoothTouchRead(int pin);

bool checkForInput();
bool checkForNight();

extern TaskHandle_t dimmingTask;
extern bool dimmingTaskRunning;
extern bool maxBrightness;
extern bool displayON;

extern bool inputDetected;


#endif
