#include "functions.h"

void centerText(String text, int y, int x)
{
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
  if (x == -1)
  {
    x = (SCREEN_WIDTH - w) / 2;
  }
  display.setCursor(x, y);
  display.print(text);
}

String getCurrentWeekdayName()
{
  const char *weekdays[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
  time_t currentTime = now();              // Get the current time
  int weekdayIndex = weekday(currentTime); // Get the day of the week (1 = Sunday, 2 = Monday, etc.)
  return weekdays[weekdayIndex - 1];       // Adjust index to match array (0 = Sunday, 1 = Monday, etc.)
}

String getWeekdayName(int weekday)
{
  const char *weekdays[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
  int weekdayIndex = weekday; // Get the day of the week (1 = Sunday, 2 = Monday, etc.)
  return weekdays[weekdayIndex - 1];       // Adjust index to match array (0 = Sunday, 1 = Monday, etc.)
}


