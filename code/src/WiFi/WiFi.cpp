#include "WiFi.h"

WiFiMulti wifiMulti;

TaskHandle_t wifiTask = NULL;
TaskStatus_t wifiTaskStatus;

void initWifi();
void WiFiEvent(WiFiEvent_t event);

bool WiFiTaskRunning = false;

bool tasksLaunched = false;
bool WifiOn = false;

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
  wifiMulti.addAP(SSID3, PASSWORD3);
}

void connectToWiFi(void *parameter)
{
  WifiOn = true;
  wifiMulti.addAP(SSID1, PASSWORD1);
  wifiMulti.addAP(SSID2, PASSWORD2);
  wifiMulti.addAP(SSID3, PASSWORD3);
  while (WifiOn == true)
  {
    WiFiTaskRunning = true;
    WiFi.mode(WIFI_STA);
    WiFi.setSleep(WIFI_PS_MAX_MODEM);
    WiFi.setAutoReconnect(true);

    if (OTAEnabled == false)
    {
      WiFi.onEvent(WiFiEvent);
    }

    Serial.println("Connecting to WiFi");
    initWifi();
    while (wifiMulti.run(17000) != WL_CONNECTED && WifiOn == true)
    {
      Serial.print(".");
      vTaskDelay(30);
    }

    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.println("\nConnected to WiFi");
      Serial.print("Got ip: ");
      Serial.println(WiFi.localIP());
      Serial.println("Mac Address: " + String(WiFi.macAddress()));
    }

    break;
  }
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

bool isWifiTaskCheck()
{
  bool tmp = WiFiTaskRunning;
  return tmp;
}

void turnOffWifiMinimal()
{
  if (WiFi.getMode() != WIFI_OFF)
  {
    if (WiFi.disconnect(true) == false)
    {
      Serial.println("Failed to disconnect from wifi? turning it off anyway");
      if (WiFi.mode(WIFI_OFF) == false)
      {
        Serial.println("Failed to force set mode of wifi, doing manual esp idf way");
      }
    }
  }
}

void turnOffWifi()
{
  WifiOn = false;
  Serial.println("Turning wifi off");
  if (WiFi.status() == WL_CONNECTED)
  {
    while (WiFiTaskRunning == true)
    {
      vTaskDelay(30);
      Serial.println("WiFi Task running watiting for it to complete");
    }

    while (WiFi.scanComplete() == WIFI_SCAN_RUNNING)
    {
      Serial.println("WiFi Scan running watiting for it to complete");
      vTaskDelay(30);
    }
    // vTaskSuspend(wifiTask);
    // delayTask(1500);
  }
  turnOffWifiMinimal();
}

void WiFiEvent(WiFiEvent_t event)
{
  Serial.println("WiFi event");
  switch (event)
  {
  case ARDUINO_EVENT_WIFI_STA_GOT_IP:
    Serial.println("WiFi got IP");
    if (tasksLaunched == false)
    {
      delay(4000);
      Serial.println("Launching Tasks");
      synchronizeAndSetTime();
      Serial.println("Synchronized Time");
      delay(1000);
      createWeatherTask();
      int currentHour = hour();
      int currentMinute = minute();
      LedDisplay.showNumberDecEx(currentHour * 100 + currentMinute, 0b11100000, true);
      tasksLaunched = true;
    }
    break;

  case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
    delay(4000);
    Serial.println("WiFi disconnected");
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    break;

  default:
    break;
  }
}
