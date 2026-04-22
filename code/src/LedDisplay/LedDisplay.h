#pragma once

#include "../defines.h"

extern bool LedTaskRunning;

extern TaskHandle_t LedTask;

extern std::mutex LedMut;

void showCurrentTime();
void createLedDisplayTask();
