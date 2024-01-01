#include "defines.h"

void setup()
{
  Serial.println("initializing Hardware");

  initHardware();
  createWifiTask();
  createDimmingTask();
  initMenus();
  readAlarms();
  display.setFont(&DejaVu_LGC_Sans_Bold_10);
}

void loop()
{
  handleMenus();
  showTime();
  display.display();
}
