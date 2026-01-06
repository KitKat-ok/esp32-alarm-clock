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
  int tete = digitalRead(TOUCH_INTERRUPT);
  Serial.println("State" + String(tete));
  AT42QT2120::Status s = touch_sensor.getStatus();

  Serial.print("keys: ");
  Serial.print(s.keys, BIN);

  Serial.print("  any: ");
  Serial.print(s.any_key_touched);

  Serial.print("  slider: ");
  Serial.print(s.slider_or_wheel);

  Serial.print("  pos: ");
  Serial.print(s.slider_or_wheel_position);

  Serial.print("  cal: ");
  Serial.println(s.calibrating);
  delay(100);
}
