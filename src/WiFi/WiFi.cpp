#include "WiFi.h"

WiFiMulti wifiMulti;

TaskHandle_t wifiTask = NULL;

void initWifi();
void WiFiEvent(WiFiEvent_t event);

bool WiFiTaskRunning = false;

bool tasksLaunched = false;

void initWiFiHandle(void *parameter)
{
  while (true)
  {
    vTaskDelete(NULL);
  }
}

void initWifi()
{

  Serial.println("Starting Wifi Task");
  xTaskCreatePinnedToCore(
      initWiFiHandle, // Task function
      "WiFiTask",     // Task name
      4096,           // Stack size
      NULL,           // Task parameters
      1,              // Priority
      &wifiTask,      // Task handle
      1);
  wifiMulti.addAP(SSID1, PASSWORD1);
  wifiMulti.addAP(SSID2, PASSWORD2);
}

void connectToWiFi(void *parameter)
{
  WiFiTaskRunning = true;
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(WIFI_PS_MAX_MODEM);
  WiFi.setAutoConnect(true);
  WiFi.setAutoReconnect(true);

  if (OTAEnabled == false)
  {
    WiFi.onEvent(WiFiEvent);
  }

  Serial.println("Connecting to WiFi");
  initWifi();
  while (wifiMulti.run(17000) != WL_CONNECTED)
  {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("\nConnected to WiFi");
  Serial.print("Got ip: ");
  Serial.println(WiFi.localIP());
  Serial.println("Mac Address: " + String(WiFi.macAddress()));

  WiFiTaskRunning = false;
  vTaskDelete(NULL);
}

void createWifiTask()
{
  xTaskCreatePinnedToCore(
      connectToWiFi, // Task function
      "WiFiTask",    // Task name
      4096,          // Stack size
      NULL,          // Task parameters
      1,             // Priority
      &wifiTask,     // Task handle
      1);            // Core (0 or 1)
}

void turnOffWifi()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    WiFi.setAutoConnect(false);
    WiFi.setAutoReconnect(false);
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    Serial.println("turning WiFi off");
  }
}

void WiFiEvent(WiFiEvent_t event)
{
  switch (event)
  {
  case SYSTEM_EVENT_STA_GOT_IP:
    Serial.println("WiFi connected");
    if (tasksLaunched == false)
    {
      delay(4000);
      Serial.println("Launching Tasks");
      synchronizeAndSetTime();
      Serial.println("Synchronized Time");
      createAlarmTask();
      Serial.println("Launched Alarm Task");
      synchronizeAndSetTime();
      delay(1000);
      createWeatherTask();
      int currentHour = hour();
      int currentMinute = minute();
      LedDisplay.showNumberDecEx(currentHour * 100 + currentMinute, 0b11100000, true);
      tasksLaunched = true;
    }
    break;

  case SYSTEM_EVENT_STA_DISCONNECTED:
    delay(4000);
    Serial.println("WiFi disconnected");
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);

    break;

  default:
    break;
  }
}