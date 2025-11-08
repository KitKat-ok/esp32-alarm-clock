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
  setupScreensaver();
  initMenus();
  createBatteryTask();
  createTempTask();
  createLightTask();
  createTimeTask();
}

void loop()
{
  delay(1000);
  int tete = digitalRead(MCP_INTERRUPT_PIN);
  Serial.println("State" + String(tete));
}
