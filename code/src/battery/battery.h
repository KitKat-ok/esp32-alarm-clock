#ifndef BATTERY_H
#define BATTERY_H

#include "../defines.h"

int getBatteryPercentage();
float getBatteryVoltage();
void createBatteryTask();

void listenToSleep();

extern int batteryPercentage;
extern bool powerConnected;
extern bool charging;
extern bool wentToSleep;
extern bool goToSleep;
extern float batteryVoltage;

#endif
