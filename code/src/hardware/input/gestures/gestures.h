#pragma once

#include "defines.h"
extern Adafruit_APDS9960 apds;


void turnOnGesture();
void resumeGestureTask();

#if GESTURES_ENABLED == true
typedef void (*GestureCallback)(int gestureType);
void registerGestureCallback(GestureCallback cb);

struct GestureState {
    bool detected = false;
    int gestureType = 0;
    uint32_t timestamp = 0;
};

GestureState useGesture();
GestureState useAllGesture();
void setGesture(const GestureState &g);

#endif // GESTURES_ENABLED