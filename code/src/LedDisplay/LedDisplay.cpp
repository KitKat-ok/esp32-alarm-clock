#include "LedDisplay.h"

int previousMinute = -1;

void showTimeTask(void *pvParameters);

TaskHandle_t LedTask;

std::mutex LedMut;

void createLedDisplayTask()
{

  Serial.print("creating Led display task");

  xTaskCreate(
      showTimeTask, /* Task function. */
      "LedTask",    /* String with name of task. */
      2048,         /* Stack size in words. */
      NULL,         /* Parameter passed as input of the task */
      1,            /* Priority of the task. */
      &LedTask      /* Task handle. */
  );
}

void showCurrentTime()
{
  LedMut.lock();
  String h = String(hour());
  String m = String(minute());
  if (hour() < 10)
    h = "0" + h;
  if (minute() < 10)
    m = "0" + m;
  LedDisplay.display((h + "." + m).c_str());
  LedMut.unlock();
}

void showTimeTask(void *pvParameters)
{
  showCurrentTime();

  while (true)
  {
    int s = second();
    if (s != 0)
      vTaskDelay(pdMS_TO_TICKS((60 - s) * 1000));

    if (LedDisplayOn)
    {
      showCurrentTime();
    }

    vTaskDelay(pdMS_TO_TICKS(60000));
  }
}
