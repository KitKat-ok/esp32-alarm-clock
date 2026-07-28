#pragma once

#include "defines.h"

#define COLOR_CHART_READINGS 10
#define COLOR_INTERVAL_MS 1000

struct ColorData {
    uint16_t ambient = 0;
    uint16_t red = 0;
    uint16_t green = 0;
    uint16_t blue = 0;
};

// Global sensor instance using Adafruit_APDS9960
extern Adafruit_APDS9960 apds;

// Function Prototypes
void initColorSensor();
bool readColorData(ColorData &data);
uint16_t readAmbientLight();

void setGestureInterrupt(bool enable);
