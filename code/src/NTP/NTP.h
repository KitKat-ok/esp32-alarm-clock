#pragma once

#include "../defines.h"

void createTimeTask();
void synchronizeAndSetTime();
void deleteTimeTask();

void syncTimeLibWithRTC();
void syncESP32RTC();

extern TaskHandle_t NTPTask;
