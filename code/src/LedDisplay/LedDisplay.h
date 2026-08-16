#pragma once

#include "../defines.h"

extern bool LedTaskRunning;

extern TaskHandle_t TimeTask;
extern TaskHandle_t LedTask;

extern AS1115 LedDisplay;

extern SemaphoreHandle_t LedMut;

void showCurrentTime();
void createLedDisplayTask();
void setLedIntensity(uint8_t target);
void initLedDisplay();

void disableLedDisplay();
void enableLedDisplay();