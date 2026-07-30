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

#else

typedef void (*ProximityCallback)(uint8_t proxValue);
void registerProximityCallback(ProximityCallback cb);

struct ProximityState {
    bool detected = false;
    uint8_t proxValue = 0;
    uint32_t timestamp = 0;
};

ProximityState useProximity();
ProximityState useAllProximity();
void setProximity(const ProximityState &p);

#endif // GESTURES_ENABLED