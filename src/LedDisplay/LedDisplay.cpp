#include "LedDisplay.h"


int previousMinute = -1;

void showTime()
{
int currentHour = hour();
int currentMinute = minute();
 if (currentMinute != previousMinute) {
    LedDisplay.showNumberDecEx(currentHour * 100 + currentMinute, 0b11100000, true);

    // Update the previous minute
    previousMinute = currentMinute;
  }
}