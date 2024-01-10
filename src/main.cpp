#include "defines.h"

void setup()
{
  Serial.println("initializing Hardware");

  initHardware();
  initBattery();
  createDimmingTask();
  delay(5000);
  createWifiTask();
  createBatteryTask();
  initMenus();
  readAlarms();
  initTemperature();
  display.setFont(&DejaVu_LGC_Sans_Bold_10);
}

void loop()
{
  handleMenus();
  showTime();
  display.display();
}
