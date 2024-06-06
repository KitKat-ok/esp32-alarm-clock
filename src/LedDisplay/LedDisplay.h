#ifndef LEDDISPLAY_H
#define LEDDISPLAY_H

#include "../defines.h"

extern bool LedTaskRunning;

extern TaskHandle_t LedTask;

void createLedDisplayTask();

#endif
