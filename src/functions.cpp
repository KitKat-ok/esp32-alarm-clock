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

String getCurrentMonthName()
{
  const char *months[] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
  time_t currentTime = now();                 // Get the current time
  int monthIndex = month(currentTime);        // Get the month (1 = January, 2 = February, etc.)
  return months[monthIndex - 1];              // Adjust index to match array (0 = January, 1 = February, etc.)
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
  int weekdayIndex = weekday;        // Get the day of the week (1 = Sunday, 2 = Monday, etc.)
  return weekdays[weekdayIndex - 1]; // Adjust index to match array (0 = Sunday, 1 = Monday, etc.)
}

String getNextDayName(int daysAfterToday)
{
  // Get the current time
  time_t currentTime = now();

  // Calculate the future time by adding days
  time_t futureTime = currentTime + (daysAfterToday * 24 * 60 * 60);

  // Get the day of the week for the future time
  int dayOfWeek = weekday(futureTime);

  // Get the name of the day
  String dayName = dayStr(dayOfWeek);

  return dayName;
}

String resetReasonToString(int reason)
{
  switch (reason)
  {
  case 1:
    return "POWERON_RESET";
    break; /**<1, Vbat power on reset*/
  case 3:
    return "SW_RESET";
    break; /**<3, Software reset digital core*/
  case 4:
    return "OWDT_RESET";
    break; /**<4, Legacy watch dog reset digital core*/
  case 5:
    return "DEEPSLEEP_RESET";
    break; /**<5, Deep Sleep reset digital core*/
  case 6:
    return "SDIO_RESET";
    break; /**<6, Reset by SLC module, reset digital core*/
  case 7:
    return "TG0WDT_SYS_RESET";
    break; /**<7, Timer Group0 Watch dog reset digital core*/
  case 8:
    return "TG1WDT_SYS_RESET";
    break; /**<8, Timer Group1 Watch dog reset digital core*/
  case 9:
    return "RTCWDT_SYS_RESET";
    break; /**<9, RTC Watch dog Reset digital core*/
  case 10:
    return "INTRUSION_RESET";
    break; /**<10, Instrusion tested to reset CPU*/
  case 11:
    return "TGWDT_CPU_RESET";
    break; /**<11, Time Group reset CPU*/
  case 12:
    return "SW_CPU_RESET";
    break; /**<12, Software reset CPU*/
  case 13:
    return "RTCWDT_CPU_RESET";
    break; /**<13, RTC Watch dog Reset CPU*/
  case 14:
    return "EXT_CPU_RESET";
    break; /**<14, for APP CPU, reseted by PRO CPU*/
  case 15:
    return "RTCWDT_BROWN_OUT_RESET";
    break; /**<15, Reset when the vdd voltage is not stable*/
  case 16:
    return "RTCWDT_RTC_RESET";
    break; /**<16, RTC Watch dog reset digital core and rtc module*/
  default:
    return "NO_MEAN";
  }
}

String wifiStatusToString(int status)
{
  switch (status)
  {
  case WL_IDLE_STATUS:
    return "Idle";
  case WL_NO_SSID_AVAIL:
    return "No SSID Available";
  case WL_SCAN_COMPLETED:
    return "Scan Completed";
  case WL_CONNECTED:
    return "Connected";
  case WL_CONNECT_FAILED:
    return "Connect Failed";
  case WL_CONNECTION_LOST:
    return "Connection Lost";
  case WL_DISCONNECTED:
    return "Disconnected";
  default:
    return "Unknown Status";
  }
}

String microsecondsToTimeString(uint64_t microseconds) {
    // Convert microseconds to seconds
    uint64_t total_seconds = microseconds / 1000000;

    // Calculate hours, minutes, and seconds
    uint64_t hours = total_seconds / 3600;
    uint64_t remaining_seconds = total_seconds % 3600;
    uint64_t minutes = remaining_seconds / 60;
    uint64_t seconds = remaining_seconds % 60;

    // Create the formatted string
    String formatted_time = String(hours) + ":" + String(minutes) + ":" + String(seconds);
    return formatted_time;
}
