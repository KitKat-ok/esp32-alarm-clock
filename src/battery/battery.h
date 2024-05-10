#ifndef BATTERY_H
#define BATTERY_H

#include "../defines.h"

int getBatteryPercentage();
void createBatteryTask();
void initBattery();

void goToSleep();

extern int batteryPercentage;
extern float charging;

#endif
