#ifndef NTP_H
#define NTP_H

#include "../defines.h"

void createTimeTask();
void synchronizeAndSetTime();
void deleteTimeTask();

extern TaskHandle_t NTPTask;

#endif
