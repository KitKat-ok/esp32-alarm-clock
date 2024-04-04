#include "LedDisplay.h"

int previousMinute = -1;

void showTimeTask(void *pvParameters);

void createLedDisplayTask()
{
  Serial.print("creating dimmingTask");
  xTaskCreatePinnedToCore(
      showTimeTask, /* Task function. */
      "DimTask",    /* String with name of task. */
      10000,        /* Stack size in words. */
      NULL,         /* Parameter passed as input of the task */
      1,            /* Priority of the task. */
      NULL,         /* Task handle. */
      0             /* Core where the task should run. */
  );
}

void showTimeTask(void *pvParameters)
{
  unsigned long previousMillis = 0; // Variable to store the last time the LED was updated
  const long interval = 1000;       // Interval at which to blink (in milliseconds)
  while (true)
  {
    while (maxBrightness == false)
    {
      unsigned long currentMillis = millis(); // Get the current time
      int currentHour = hour();
      int currentMinute = minute();
      if (currentMillis - previousMillis >= interval)
      {
        if (charging == true)
        {
          dimLedDisplay();
        }
      }
      if (currentMinute != previousMinute)
      {
        if (displayON == true && (currentHour >= 23 || currentHour < 10))
        {
          LedDisplay.showNumberDecEx(currentHour * 100 + currentMinute, 0b11100000, true);
        }

        // Update the previous minute
        previousMinute = currentMinute;
      }
      vTaskDelay(10);
    }

    int currentHour = hour();
    int currentMinute = minute();
    LedDisplay.setBrightness(7);
    LedDisplay.showNumberDecEx(currentHour * 100 + currentMinute, 0b11100000, true);
    while (maxBrightness == true)
    {
      vTaskDelay(10);
    }
    dimLedDisplay();
    LedDisplay.showNumberDecEx(currentHour * 100 + currentMinute, 0b11100000, true);
  }
}