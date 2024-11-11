#ifndef ALARMS_H
#define ALARMS_H

#include "../defines.h"

void onAlarm();
void createAlarmTask();
void ringAlarm(void *parameter);

void disableAllAlarms();
void enableAllAlarms();  // <-- Added semicolon here

void initialzeAlarmArray();

struct Alarm {
    bool exists;
    bool enabled;
    int day;
    int hours;
    int minutes;
    bool soundOn;
};

extern Alarm alarms[MAX_ALARMS];  // Declaration of the array

#endif
