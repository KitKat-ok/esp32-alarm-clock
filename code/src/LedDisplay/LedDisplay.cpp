#include "LedDisplay.h"

int previousMinute = -1;

void showTimeTask(void *pvParameters);

TaskHandle_t LedTask;

void createLedDisplayTask()
{

  Serial.print("creating Led display task");

  xTaskCreatePinnedToCore(
      showTimeTask, /* Task function. */
      "LedTask",    /* String with name of task. */
      2048,         /* Stack size in words. */
      NULL,         /* Parameter passed as input of the task */
      1,            /* Priority of the task. */
      &LedTask,     /* Task handle. */
      0             /* Core where the task should run. */
  );
}

void showTimeTask(void *pvParameters)
{
  unsigned long previousMillis = 0;
  const long interval = 1000; // 10 seconds interval
  while (true)
  {
    while (maxBrightness == false)
    {
      unsigned long currentMillis = millis(); // Get the current time
      if (currentMillis - previousMillis >= interval)
      {
        previousMillis = currentMillis;

        Serial.println("Reading brightness and dimming Led display accordingly");

        dimLedDisplay();

        if (displayON == true)
        {
          LedDisplay.showNumberDecEx(hour() * 100 + minute(), 0b11100000, true);
        }
      }
      vTaskDelay(10);
    }

    Serial.println("Setting max brightness Led display");

    if (hour() >= 23 || hour() < 10)
    {
      LedDisplay.setBrightness(2);
      LedDisplay.showNumberDecEx(hour() * 100 + minute(), 0b11100000, true);
      while (maxBrightness == true)
      {
        LedDisplay.showNumberDecEx(hour() * 100 + minute(), 0b11100000, true);
        vTaskDelay(1000);
      }
      dimLedDisplay();
      LedDisplay.showNumberDecEx(hour() * 100 + minute(), 0b11100000, true);
    }
    else
    {
      LedDisplay.setBrightness(7);
      LedDisplay.showNumberDecEx(hour() * 100 + minute(), 0b11100000, true);
      while (maxBrightness == true)
      {

        LedDisplay.showNumberDecEx(hour() * 100 + minute(), 0b11100000, true);
        vTaskDelay(1000);
      }
      dimLedDisplay();
      LedDisplay.showNumberDecEx(hour() * 100 + minute(), 0b11100000, true);
    }
  }
}