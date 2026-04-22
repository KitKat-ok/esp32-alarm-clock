#pragma once

#include "defines.h"

void createTempTask();

float readHumidity();
float readTemperature();

void initTempSensor();

extern Adafruit_SHT4x sht4;

