#ifndef BATTERY_H
#define BATTERY_H

#include "../defines.h"

int getBatteryPercentage();
void createBatteryTask();

void goToSleep();

extern int batteryPercentage;
extern bool charging;
extern float batteryVoltage;

#endif
