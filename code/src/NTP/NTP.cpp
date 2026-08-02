#include "NTP.h"

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
TaskHandle_t NTPTask;

// Timezones
TimeChangeRule myDST = {"CEST", Last, Sun, Mar, 2, 120}; // UTC+2
TimeChangeRule mySTD = {"CET", Last, Sun, Oct, 3, 60};   // UTC+1

Timezone myTZ(myDST, mySTD);
TimeChangeRule *tcr;

void syncTimeTask(void *parameter);

void createTimeTask()
{
  xTaskCreate(
      syncTimeTask,
      "SyncTimeTask",
      2048,
      NULL,
      1,
      &NTPTask);
}

void deleteTimeTask()
{
  vTaskDelete(NTPTask);
}

void syncTimeLibWithRTC()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  setTime(tv.tv_sec);
  Serial.println("TimeLib synchronized with internal RTC");
}

void syncESP32RTC()
{
  time_t now = ::now();
  struct timeval tv;
  tv.tv_sec = now;
  tv.tv_usec = 0;
  settimeofday(&tv, NULL);
  Serial.println("Internal RTC synchronized with TimeLib");
}

void syncTimeTask(void *parameter)
{
  bool syncedSuccessfully = false;

  while (true)
  {
    syncedSuccessfully = synchronizeAndSetTime();

    if (syncedSuccessfully)
    {
      // Normal interval: wait 10 minutes when synced
      vTaskDelay(pdMS_TO_TICKS(10 * 60 * 1000));
    }
    else
    {
      // Retry interval: wait 1 minute if failed or offline
      Serial.println("Sync attempt unsuccessful. Retrying in 1 minute...");
      vTaskDelay(pdMS_TO_TICKS(1 * 60 * 1000));
    }
  }
}

bool synchronizeAndSetTime()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("WiFi not connected. Cannot sync time.");
    return false;
  }

  Serial.println("Synchronizing Time...");
  timeClient.begin();

  // forceUpdate returns true if valid packet received from NTP server
  bool success = timeClient.forceUpdate();

  if (success)
  {
    time_t utc = timeClient.getEpochTime();

    // Verify epoch time is valid (greater than Jan 1, 2024 timestamp)
    if (utc > 1704067200)
    {
      time_t local = myTZ.toLocal(utc, &tcr);
      setTime(local);
      timeClient.end();
      syncESP32RTC();
      Serial.println("Current time: " + String(hour()) + ":" + String(minute()) + " " + tcr->abbrev);
      return true;
    }
  }

  timeClient.end();
  Serial.println("NTP update failed or received invalid time.");
  return false;
}