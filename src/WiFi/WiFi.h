#ifndef WIFI_H
#define WIFI_H

#include "../defines.h"

void createWifiTask();
void turnOffWifi();
void initWifi();

extern TaskHandle_t wifiTask;

extern bool WiFiTaskRunning;


#endif
