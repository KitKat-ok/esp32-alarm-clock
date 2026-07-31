#include "LedDisplay.h"

uint8_t currentIntensity;

void showTimeTask(void *pvParameters);
void LedDisplayTask(void *pvParameters);

TaskHandle_t TimeTask;
TaskHandle_t LedTask;

std::mutex LedMut;

void createLedDisplayTask()
{

  Serial.print("creating Led display task");

  xTaskCreate(
      showTimeTask, /* Task function. */
      "TimeTask",   /* String with name of task. */
      2048,         /* Stack size in words. */
      NULL,         /* Parameter passed as input of the task */
      1,            /* Priority of the task. */
      &TimeTask     /* Task handle. */
  );

  xTaskCreate(
      LedDisplayTask, /* Task function. */
      "LedTask",      /* String with name of task. */
      2048,           /* Stack size in words. */
      NULL,           /* Parameter passed as input of the task */
      0,              /* Priority of the task. */
      &LedTask        /* Task handle. */
  );
}

uint8_t targetIntensity;
uint32_t lastUpdate = 0;

void setLedIntensity(uint8_t target)
{
  targetIntensity = target;
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
  if (lockI2C())
  {
    LedDisplay.display((h + "." + m).c_str());
    unlockI2C();
  }
  LedMut.unlock();
}

void updateIntensity()
{
  int step = (targetIntensity > currentIntensity) ? 1 : -1;
  currentIntensity = targetIntensity;
  for (int i = currentIntensity; i != targetIntensity; i += step)
  {
    if (lockI2C())
    {
      LedDisplay.setIntensity(i);
      LedMut.unlock();

      delay(30);
    }
  }

  if (lockI2C())
  {
    LedDisplay.setIntensity(currentIntensity);
    unlockI2C();
  }
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

bool lastLedState = false;

void LedDisplayTask(void *pvParameters)
{
  while (true)
  {
    if (LedDisplayOn == true && lastLedState == false)
    {
      showCurrentTime();
      delay(10);
    }
    lastLedState = LedDisplayOn;

    if (LedDisplayOn == true)
    {
      updateIntensity();
    }

    vTaskDelay(pdMS_TO_TICKS(100));
  }
}
