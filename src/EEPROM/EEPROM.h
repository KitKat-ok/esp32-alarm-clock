#ifndef EEPROM_H
#define EEPROM_H

#include "../defines.h"

struct AlarmEntry {
  int id;
  int day;
  int hour;
  int minute;
};

const int maxAlarms = 7;
extern AlarmEntry alarms[maxAlarms];



#endif
