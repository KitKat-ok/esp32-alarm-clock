#include "NTP.h"

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
TaskHandle_t NTPTask;

// Define your timezone and DST rules
const int TIME_ZONE = 1; // Poland is UTC+1
bool isDST = false; // Initialize DST status

// Function to check if DST is in effect for the current date
bool isDaylightSavingTime() {
  // Get the current year, month, and day
  int currentYear = year();
  int currentMonth = month();
  int currentDay = day();

  // Calculate the last Sunday in March
  int lastSundayMarch = 31 - (5 + (currentYear * 5 / 4 + 4) % 7);

  // Calculate the last Sunday in October
  int lastSundayOctober = 31 - (5 + (currentYear * 5 / 4 + 2) % 7);

  // Check if the current date falls within the DST period
  if ((currentMonth > 3 || (currentMonth == 3 && currentDay >= lastSundayMarch)) &&
      (currentMonth < 10 || (currentMonth == 10 && currentDay < lastSundayOctober))) {
    return true;
  }
  return false;
}

void syncTimeTask(void *parameter);

void createTimeTask() {
  xTaskCreatePinnedToCore(
      syncTimeTask,   // Function to implement the task
      "SyncTimeTask", // Name of the task
      10000,          // Stack size (words)
      NULL,           // Parameter to pass
      1,              // Priority
      &NTPTask,       // Task handle
      1               // Core to run the task on (Core 1)
  );
}

void deleteTimeTask() {
  vTaskDelete(NTPTask);
}

void syncTimeTask(void *parameter) {
  while (true) {
    // Check if WiFi is connected
    if (WiFi.status() == WL_CONNECTED) {
      synchronizeAndSetTime();
    } else {
      // WiFi is not connected, print a message or take appropriate action
      Serial.println("WiFi not connected. Cannot sync time.");
    }

    // Wait for 10 minutes before the next synchronization
    vTaskDelay(pdMS_TO_TICKS(10 * 60 * 1000));
  }
}

void synchronizeAndSetTime() {
  Serial.println("Synchronizing Time");
  timeClient.begin();
  timeClient.update();
  
  // Check if DST is in effect
  if (isDaylightSavingTime()) {
    isDST = true;
  } else {
    isDST = false;
  }

  // Adjust time offset for DST
  if (isDST) {
    timeClient.setTimeOffset(TIME_OFFSET_S + 3600); // Add one hour for DST
  } else {
    timeClient.setTimeOffset(TIME_OFFSET_S);
  }

  setTime(timeClient.getEpochTime());  // Set the system time
  timeClient.end();
  Serial.println("Current time " + String(hour()) + ":" + String(minute()));
}
