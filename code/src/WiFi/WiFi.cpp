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
      2,              // Priority
      &wifiTask,      // Task handle
      0);
}

#define SIZE_WIFI_CRED_STAT 3

WiFiCred *wifiCredStatic[3];

#define WIFI_SYNC_TIME 50000

#define WIFI_COUNTRY_FIX 1 // Enable this to 1 to enable the fix
/*
Supported country codes are "01"(world safe mode) "AT","AU","BE","BG","BR", "CA","CH","CN","CY","CZ","DE","DK","EE","ES","FI","FR","GB","GR","HK","HR","HU", "IE","IN","IS","IT","JP","KR","LI","LT","LU","LV","MT","MX","NL","NO","NZ","PL","PT", "RO","SE","SI","SK","TW","US"
*/
#define WIFI_COUNTRY_CODE "PL"
#define WIFI_COUNTRY_FORCE false // This should be false, you can set it to true to check if something starts working

void setWifiCountryCode()
{
#if WIFI_COUNTRY_FIX
  Serial.println("Setting wifi country code to: " + String(WIFI_COUNTRY_CODE));
  esp_wifi_set_country_code(WIFI_COUNTRY_CODE, !WIFI_COUNTRY_FORCE);
#endif
}

void tryToConnectWifi()
{

  for (int i = 0; i < SIZE_WIFI_CRED_STAT; i++)
  {
    if (wifiCredStatic[i] == NULL || wifiCredStatic[i]->ssid == NULL || wifiCredStatic[i]->password == NULL)
    {
      continue;
    }
    else if (strlen(wifiCredStatic[i]->ssid) == 0 || strlen(wifiCredStatic[i]->password) < 8)
    {
      continue;
    }

    vTaskDelay(100 / portTICK_PERIOD_MS);

    // Configure Wi-Fi with the current credentials
    wifi_config_t wifi_config = {};
    strncpy((char *)wifi_config.sta.ssid, wifiCredStatic[i]->ssid, sizeof(wifi_config.sta.ssid));
    strncpy((char *)wifi_config.sta.password, wifiCredStatic[i]->password, sizeof(wifi_config.sta.password));

    setWifiCountryCode();

    wifi_scan_config_t scan_config = {};
    scan_config.scan_time.active.min = 500;  // Increase minimum active scan time (ms)
    scan_config.scan_time.active.max = 2000; // Increase maximum active scan time (ms)

    esp_wifi_scan_start(&scan_config, false);
    // Disconnect any existing connections before attempting to connect with new credentials
    esp_wifi_disconnect();

    // Set the Wi-Fi configuration
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);

    // Connect to the Wi-Fi network
    esp_wifi_connect();

    int attempts = WIFI_SYNC_TIME / 1000;
    for (int j = 0; j < attempts; j++)
    {
      vTaskDelay(1000 / portTICK_PERIOD_MS);
      wifi_ap_record_t ap_info;
      esp_wifi_sta_get_ap_info(&ap_info);
      if (ap_info.rssi != 0)
      {
        Serial.println("Connected to WiFi");
        return;
      }
      else
      {
        Serial.println("Failed to connect to WiFi...");
      }
    }

    wifi_ap_record_t ap_info;
    esp_wifi_sta_get_ap_info(&ap_info);
    if (ap_info.rssi != 0)
    {
      Serial.println("Connected to WiFi");
      return;
    }
  }
}

void connectToWiFi(void *parameter)
{
  WifiOn = true;

  // Initialize wifiCredStatic array
  while (WifiOn)
  {
    WiFi.setSleep(WIFI_PS_MAX_MODEM);
    WiFi.setAutoReconnect(true);

    WiFiTaskRunning = true;

    esp_wifi_start();

    if (!OTAEnabled && tasksLaunched == false)
    {
      WiFi.onEvent(WiFiEvent);
    }

    Serial.println("Connecting to WiFi");

    // Add predefined WiFi credentials
    wifiCredStatic[0] = new WiFiCred{SSID1, PASSWORD1};
    wifiCredStatic[1] = new WiFiCred{SSID2, PASSWORD2};
    wifiCredStatic[2] = new WiFiCred{SSID3, PASSWORD3};

    // Call tryToConnectWifi to handle the connection attempts
    tryToConnectWifi();

    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.println("\nConnected to WiFi");
      Serial.print("Got IP: ");
      Serial.println(WiFi.localIP());
      Serial.println("Mac Address: " + String(WiFi.macAddress()));
      break; // Exit the loop after successful connection
    }
    else
    {
      Serial.println("Failed to connect to any WiFi network. Retrying...");
    }

    vTaskDelay(30000 / portTICK_PERIOD_MS); // Wait before retrying
  }

  WiFiTaskRunning = false;
  vTaskDelete(NULL);
}

void createWifiTask()
{
  wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();

  esp_err_t ret = esp_wifi_init(&wifi_init_config);
  xTaskCreatePinnedToCore(
      connectToWiFi, // Task function
      "WiFiTask",    // Task name
      10000,         // Stack size
      NULL,          // Task parameters
      10,            // Priority
      &wifiTask,     // Task handle
      0);            // Core (0 or 1)
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
    // delay(1500);
  }
  esp_wifi_stop();
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
    break;

  default:
    break;
  }
}
