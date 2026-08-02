#pragma once

#include "../../defines.h"

void currentWeather();
void displayWeatherCast(int dayIndex);

String convertWindDirection(uint16_t degrees);

void checkExit();

void updateWeatherMenuDates();

void initWeatherMenu();
void currentWeatherMenu();
void wifiDebugMenu();
void CPUDebugMenu();
void generalDebugMenu();
void fpsCalc();
