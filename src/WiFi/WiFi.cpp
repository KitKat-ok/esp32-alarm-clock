#include "WiFi.h"

WiFiMulti wifiMulti;

TaskHandle_t wifiTask;

void initWifi();
void WiFiEvent(WiFiEvent_t event);

bool WiFiTaskRunning = false;

bool tasksLaunched = false;

void initWifi()
{

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
  WiFi.onEvent(WiFiEvent);

  Serial.println("Connecting to WiFi");
  initWifi();
  while (wifiMulti.run(17000) != WL_CONNECTED)
  {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("\nConnected to WiFi");

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
  
  if (tasksLaunched == true)
  {
      Serial.println("turning NTP off");
      WiFi.disconnect(true);
      WiFi.mode(WIFI_OFF);
      deleteTimeTask();
      deleteWeatherTask();
      tasksLaunched = false;
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
      createWeatherTask();
      synchronizeAndSetTime();
      createAlarmTask();
      createTimeTask();
      tasksLaunched = true;
    }
    break;

  case SYSTEM_EVENT_STA_DISCONNECTED:
    Serial.println("WiFi disconnected");
    break;

  default:
    break;
  }
}