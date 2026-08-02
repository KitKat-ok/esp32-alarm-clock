#pragma once

#include "../defines.h"

void createWifiTask();
void turnOffWifi();
void initWifi();

extern TaskHandle_t wifiTask;

extern bool WifiTaskRunning;

extern bool tasksLaunched;

void turnOffWifiMinimal();

typedef struct {
    const char* ssid;
    const char* password;
} WiFiCred;
