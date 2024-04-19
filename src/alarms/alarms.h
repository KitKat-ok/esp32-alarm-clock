#ifndef ALARMS_H
#define ALARMS_H

#include "../defines.h"

void onAlarm();
void createAlarmTask();
void ringAlarm(void *parameter);

void disableAllAlarms();
void enableAllAlarms();  // <-- Added semicolon here

struct Day {
    bool isSet;
    int hours;
    int minutes;
    bool soundOn;
};

extern Day days[7];  // Declaration of the array

#endif
