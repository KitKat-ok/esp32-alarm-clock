#ifndef LEDDISPLAY_H
#define LEDDISPLAY_H

#include "../defines.h"

extern bool LedTaskRunning;

extern TaskHandle_t LedTask;

extern std::mutex LedMut;

void showCurrentTime();
void createLedDisplayTask();

#endif
