#ifndef LIGHTSENSOR_H
#define LIGHTSENSOR_H

#include "../defines.h"

void createDimmingTask();

void dimLedDisplay();

float removeLightNoise();

bool checkForInput();
bool checkForNight();

void oledFadein();
void oledFadeout();

extern TaskHandle_t dimmingTask;
extern bool dimmingTaskRunning;
extern bool maxBrightness;
extern bool displayON;

extern bool fading;

extern bool inputDetected;


#endif
