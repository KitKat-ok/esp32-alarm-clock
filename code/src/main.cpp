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
  createBatteryTask();
  createTempTask();
  createPressureTask();
  createLightTask();
  oled.invertDisplay(false);
}

void loop()
{
  delay(10000);
}
