#ifndef MAINPAGE_H
#define MAINPAGE_H

#include "defines.h"

void showMainPage();
void showScreensaver();
void setupScreensaver();
extern bool showMainPageFlag;
extern bool displayedWeather;
extern void turnOffScreensaver();
void cyclePages();

void showFirstPage();
void showForecastPage();
void showInfoPage();
void showSensorPage();
#endif
