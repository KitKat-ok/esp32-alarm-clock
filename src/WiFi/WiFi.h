#ifndef WIFI_H
#define WIFI_H

#include "../defines.h"

void createWifiTask();
void turnOffWifi();

extern TaskHandle_t wifiTask;

extern bool WiFiTaskRunning;


#endif
