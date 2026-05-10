#include "defines.h"

void setup()
{
  initHardware();
  initWifi();
  checkForRunOta();
  createLedDisplayTask();
  createDimmingTask();
  initialzeAlarmArray();
  readAlarms();
  createAlarmTask();
  initMenus();
  createBatteryTask();
  createTempTask();
  createPressureTask();
  createLightTask();
  createTimeTask();
  oled.invertDisplay(false);
}

void loop()
{
  delay(10000);
}
