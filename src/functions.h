#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "defines.h"

void centerText(String text, int y, int x = -1);
String getCurrentWeekdayName();

String getWeekdayName(int weekday);

String getNextDayName(int daysAfterToday);

String getCurrentMonthName();
String getMonthName(int monthNumber);


String resetReasonToString(int reason);

int getChipRevision();

String microsecondsToTimeString(uint64_t microseconds);

String wifiStatusToString(int status);


#endif
