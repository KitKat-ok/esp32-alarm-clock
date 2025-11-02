#include "LedDisplay.h"

int previousMinute = -1;

void showTimeTask(void *pvParameters);

TaskHandle_t LedTask;

void createLedDisplayTask()
{

  Serial.print("creating Led display task");

  xTaskCreate(
      showTimeTask, /* Task function. */
      "LedTask",    /* String with name of task. */
      2048,         /* Stack size in words. */
      NULL,         /* Parameter passed as input of the task */
      1,            /* Priority of the task. */
      &LedTask     /* Task handle. */
  );
}

void showCurrentTime() {
  LedDisplay.display(String(String(hour()) + "." + String(minute())).c_str());
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
          showCurrentTime();
        }
      }
      vTaskDelay(10);
    }

    Serial.println("Setting max brightness Led display");

    if (hour() >= 23 || hour() < 10)
    {
      LedDisplay.setIntensity(2);
      showCurrentTime();
      while (maxBrightness == true)
      {
        showCurrentTime();
        vTaskDelay(1000);
      }
      dimLedDisplay();
      showCurrentTime();
    }
    else
    {
      LedDisplay.setIntensity(7);
      showCurrentTime();
      while (maxBrightness == true)
      {

        showCurrentTime();
        vTaskDelay(1000);
      }
      dimLedDisplay();
      showCurrentTime();
    }
  }
}