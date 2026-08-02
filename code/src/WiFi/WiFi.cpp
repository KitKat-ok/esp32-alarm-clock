#include "WiFi.h"
#include <mutex>

WiFiMulti wifiMulti;

// Thread safety Mutex & Task Handle
std::mutex wifiTaskMutex;
TaskHandle_t wifiTask = NULL;

// Forward declarations
void WiFiEvent(WiFiEvent_t event);
void turnOffWifi();
void turnOffWifiMinimal();
bool isWifiTaskCheck();

bool WifiTaskRunning = false;
bool tasksLaunched = false;
bool WifiOn = false;

WiFiCred *wifiCredStatic[SIZE_WIFI_CRED_STAT];

void setWifiCountryCode()
{
#if WIFI_COUNTRY_FIX
  Serial.println("Setting wifi country code to: " + String(WIFI_COUNTRY_CODE));
  esp_wifi_set_country_code(WIFI_COUNTRY_CODE, !WIFI_COUNTRY_FORCE);
#endif
}

bool isWifiTaskCheck()
{
  std::lock_guard<std::mutex> lock(wifiTaskMutex);
  return WifiTaskRunning;
}

void tryToConnectWifi()
{
  Serial.println("sizeof(wifiCredStatic): " + String(SIZE_WIFI_CRED_STAT));
  for (int i = 0; i < SIZE_WIFI_CRED_STAT; i++)
  {
    if (wifiCredStatic[i] == NULL || wifiCredStatic[i]->ssid == NULL || wifiCredStatic[i]->password == NULL)
    {
      Serial.println("Skipping wifi id: " + String(i) + " because of null");
      continue;
    }
    else if (strlen(wifiCredStatic[i]->ssid) == 0 || strlen(wifiCredStatic[i]->password) < 8)
    {
      Serial.println("Skipping wifi id: " + String(i) + " because bad length");
      continue;
    }

    Serial.println("Trying to connect to wifi number: " + String(i) + " so: " + String(wifiCredStatic[i]->ssid));
    
    // Clean radio state before initiating session
    WiFi.disconnect(false);
    vTaskDelay(pdMS_TO_TICKS(100));

    setWifiCountryCode();

    wifi_config_t sta_config;
    if (esp_wifi_get_config(WIFI_IF_STA, &sta_config) == ESP_OK)
    {
      sta_config.sta.listen_interval = 1;
      esp_wifi_set_config(WIFI_IF_STA, &sta_config);
    }

    WiFi.begin(wifiCredStatic[i]->ssid, wifiCredStatic[i]->password);
    
    // Maintain full radio power during active negotiation
    WiFi.setSleep(WIFI_PS_NONE);
    WiFi.setTxPower(WIFI_POWER_19_5dBm);

    for (int t = 0; t < WIFI_SYNC_TIME / 1000; t++)
    {
      vTaskDelay(pdMS_TO_TICKS(1000));
      if (WiFi.status() == WL_CONNECTED)
      {
        return;
      }
      else
      {
        Serial.println("Failed to connect to wifi...");
      }
    }
  }
}

void connectToWiFi(void *parameter)
{
  WifiOn = true;

  {
    std::lock_guard<std::mutex> lock(wifiTaskMutex);
    WifiTaskRunning = true;
  }

  // Allocate credentials statically once
  if (wifiCredStatic[0] == NULL) {
    wifiCredStatic[0] = new WiFiCred{SSID1, PASSWORD1};
    wifiCredStatic[1] = new WiFiCred{SSID2, PASSWORD2};
    wifiCredStatic[2] = new WiFiCred{SSID3, PASSWORD3};
  }

  WiFi.mode(WIFI_STA);
  esp_wifi_set_max_tx_power(84);
  WiFi.setAutoReconnect(true);

  if (readOtaValue() == false && tasksLaunched == false)
  {
    WiFi.onEvent(WiFiEvent);
  }

  while (WifiOn)
  {
    Serial.println("Connecting to WiFi");

    tryToConnectWifi();

    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.println("\nConnected to WiFi");
      Serial.print("Got IP: ");
      Serial.println(WiFi.localIP());
      Serial.println("Mac Address: " + String(WiFi.macAddress()));
      IPAddress subnet = WiFi.subnetMask();
      Serial.print("NETMASK: ");
      Serial.println(subnet);
      break; 
    }
    else
    {
      Serial.println("Failed to connect to any WiFi network. Retrying...");
      turnOffWifiMinimal();
    }

    vTaskDelay(pdMS_TO_TICKS(30000));
  }

  {
    std::lock_guard<std::mutex> lock(wifiTaskMutex);
    WifiTaskRunning = false;
    wifiTask = NULL;
  }

  vTaskDelete(NULL);
}

void createWifiTask()
{
  Serial.println("Creating wifi task");
  
  if (!isWifiTaskCheck())
  {
    Serial.println("xTaskCreate wifi");
    
    {
      std::lock_guard<std::mutex> lock(wifiTaskMutex);
      WifiTaskRunning = true;
    }

    xTaskCreate(
        connectToWiFi, 
        "WiFiTask",    
        4096,          
        NULL,          
        10,            
        &wifiTask      
    );
  }
  else
  {
    Serial.println("The task is already running? Skipping creation...");
  }
}

void turnOffWifiMinimal()
{
  if (WiFi.getMode() != WIFI_OFF)
  {
    if (!WiFi.disconnect(true))
    {
      Serial.println("Failed to disconnect from wifi? turning it off anyway");
      if (!WiFi.mode(WIFI_OFF))
      {
        Serial.println("Failed to force set mode of wifi");
      }
    }
  }
}

void turnOffWifi()
{
  Serial.println("Turning wifi off");
  WifiOn = false;

  if (isWifiTaskCheck())
  {
    while (WiFi.scanComplete() == WIFI_SCAN_RUNNING)
    {
      vTaskDelay(pdMS_TO_TICKS(30));
    }

    std::lock_guard<std::mutex> lock(wifiTaskMutex);
    if (WifiTaskRunning)
    {
      if (wifiTask != NULL && eTaskGetState(wifiTask) != eDeleted)
      {
        vTaskDelete(wifiTask);
        wifiTask = NULL;
      }
      WifiTaskRunning = false;
    }
  }

  turnOffWifiMinimal();
}

void WiFiEvent(WiFiEvent_t event)
{
  Serial.print("WiFi event received: ");
  Serial.println(event);

  switch (event)
  {
  case ARDUINO_EVENT_WIFI_STA_GOT_IP:
    Serial.println("-> Event: ARDUINO_EVENT_WIFI_STA_GOT_IP");
    if (!tasksLaunched)
    {
      vTaskDelay(pdMS_TO_TICKS(4000));
      Serial.println("Launching Tasks");
      Serial.println("Synchronized Time");
      vTaskDelay(pdMS_TO_TICKS(1000));
      createWeatherTask();
      createTimeTask();
      showCurrentTime();
      tasksLaunched = true;
    }
    break;

  case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
  case ARDUINO_EVENT_PROV_CRED_FAIL:
  case ARDUINO_EVENT_WIFI_STA_LOST_IP:
    Serial.println("-> Event: WiFi Disconnected/Lost IP");
    if (powerConnected && !isWifiTaskCheck())
    {
      turnOffWifi();
      delay(1000);
      if (!WiFi.isConnected())
      {
        esp_wifi_start();
        createWifiTask();
      }
    }
    break;

  default:
    break;
  }
}