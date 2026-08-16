#pragma once

#include "defines.h"

void createTempTask();

float readHumidity();
float readTemperature();

void initTempSensor();

void enableTempSensor();
void disableTempSensor();

extern Adafruit_SHT4x sht4;

