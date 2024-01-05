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
  if (WiFi.isConnected())
  {
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
  }
  
  if (tasksLaunched == true)
  {
    eTaskState NTPtaskState = eTaskGetState(NTPTask);
    if (NTPtaskState == eRunning && WiFi.isConnected() || NTPtaskState == eBlocked && WiFi.isConnected())
    {
      Serial.println("turning NTP off");
      WiFi.disconnect(true);
      WiFi.mode(WIFI_OFF);
      deleteTimeTask();
    }
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