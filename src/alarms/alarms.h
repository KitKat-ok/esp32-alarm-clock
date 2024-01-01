#ifndef ALARMS_H
#define ALARMS_H

#include "../defines.h"

void onAlarm();
void createAlarmTask();
void ringAlarm(void *parameter);

extern bool sunday;
extern bool monday;
extern bool tuesday;
extern bool wednesday;
extern bool thursday;
extern bool friday;
extern bool saturday;


extern int hoursSunday;
extern int minutesSunday;
extern int hoursMonday;
extern int minutesMonday;
extern int hoursTuesday;
extern int minutesTuesday;
extern int hoursWednesday;
extern int minutesWednesday;
extern int hoursThursday;
extern int minutesThursday;
extern int hoursFriday;
extern int minutesFriday;
extern int hoursSaturday;
extern int minutesSaturday;


#endif
