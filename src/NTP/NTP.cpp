#include "NTP.h"

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
TaskHandle_t NTPTask;

void syncTimeTask(void *parameter);


void createTimeTask() {
  xTaskCreatePinnedToCore(
      syncTimeTask,   // Function to implement the task
      "SyncTimeTask", // Name of the task
      10000,          // Stack size (words)
      NULL,           // Parameter to pass
      1,              // Priority
      &NTPTask,           // Task handle
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
    Serial.println("synchronizing Time");
    timeClient.begin();
    timeClient.update();
    timeClient.setTimeOffset(TIME_OFFSET_S);
    setTime(timeClient.getEpochTime());  // Set the system time
    timeClient.end();
    Serial.println("current time " + String(hour()) + ":" + String(minute()));
}