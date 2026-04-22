#pragma once

#include "defines.h"

void createDimmingTask();
void createLightTask();

void dimLedDisplay();

float getLightLevel();
void initLightSensor();
extern SparkFun_Ambient_Light lightMeter;
int smoothTouchRead(int pin);

bool checkForNight();

int getMmwaveState();

extern TaskHandle_t dimmingTaskHandle;
extern TaskHandle_t oledWakeupTaskHandle;
extern bool dimmingTaskRunning;
extern bool maxBrightness;
extern bool LedDisplayOn;

extern bool inputDetected;
extern float lightLevel;


