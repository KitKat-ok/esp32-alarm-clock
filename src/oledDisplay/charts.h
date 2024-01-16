#ifndef CHARTS_H
#define CHARTS_H

#include "../defines.h"

void loopTempGraph();
void initTempGraph();

void loopLightGraph();
void initLightGraph();

extern float temperatureArray[TEMP_CHART_READINGS];
extern float lightArray[TEMP_CHART_READINGS];

#endif
