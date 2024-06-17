#ifndef MENUS_H
#define MENUS_H

#include "defines.h"

void currentWeather();
void displayWeatherCast(int dayIndex);

void displaywidget(int code_no);
String convertWindDirection(uint16_t degrees);

void wifiDebugMenu();
void CPUDebugMenu();
void generalDebugMenu();

void showCalendar(int monthNumber, int yearNumber);
void loopCallendar();

#endif
