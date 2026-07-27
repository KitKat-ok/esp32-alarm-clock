#ifndef GESTURE_SENSOR_H
#define GESTURE_SENSOR_H

#include "defines.h"

struct GestureState {
    bool detected = false;
    int gestureType = DIR_NONE;
    uint32_t timestamp = 0;
};

extern SparkFun_APDS9960 apds;
extern bool inputDetected;

// Callback type definition for gesture events
typedef void (*GestureCallback)(int gestureType);

// Function Prototypes
void initGestureSensor();
void turnOnGesture();
GestureState useGesture();
GestureState useAllGesture();
void setGesture(const GestureState &g);
void registerGestureCallback(GestureCallback cb);

void resumeGestureTask();

#endif // GESTURE_SENSOR_H