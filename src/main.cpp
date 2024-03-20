#include "defines.h"

void setup()
{
  Serial.println("initializing Hardware");

  initHardware();
  initBattery();
  initWifi();
  createDimmingTask();
  createBatteryTask();
  initMenus();
  readAlarms();
  createTempTask();
  display.setFont(&DejaVu_LGC_Sans_Bold_10);
}

bool initialMenuRunning = false;

void loop()
{
  showTime();
  if (menuRunning != initialMenuRunning) {
    Serial.println("Menu state has changed.");
    display.clearDisplay();
    display.display();
    delay(1000);
    wakeUpMenu();
    menuRunning = false;
    initialMenuRunning = menuRunning;  
  }
  if (menuRunning == false)
  {
    handleMenus();
    display.display();
  }
  
}
