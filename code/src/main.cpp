#include "defines.h"

void setup()
{
  initHardware();
  createWifiTask();
  checkForRunOta();
  createLedDisplayTask();
  createDimmingTask();
  initialzeAlarmArray();
  readAlarms();
  createAlarmTask();
  initMenus();
  createTempTask();
  createPressureTask();
  createLightTask();
  createBatteryTask();
  oled.invertDisplay(false);
}

void loop()
{
  delay(10000);
}
