#ifndef MENUS_H
#define MENUS_H

#include "defines.h"

void currentWeather();
void displayWeatherCast(int dayIndex);

void wifiDebugMenu();
void CPUDebugMenu();
void generalDebugMenu();

void showCalendar(int monthNumber, int yearNumber);
void loopCallendar();

#endif
