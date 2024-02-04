#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "defines.h"

void centerText(String text, int y, int x = -1);
String getCurrentWeekdayName();

String getWeekdayName(int weekday);

String getNextDayName(int daysAfterToday);

#endif
