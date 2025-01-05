#include "defines.h"
#include <ArduinoOTA.h>

bool OTAEnabled = false;

timeval tv;

void setup()
{
  Serial.println("Initializing Hardware");

  initHardware();
  WiFi.mode(WIFI_STA);
  initWifi();
  readOtaValue();

  syncTimeLibWithRTC();
  LedDisplay.showNumberDecEx(hour() * 100 + minute(), 0b11100000, true);

  // Check if both buttons are pressed to enable OTA
  int upButtonState = digitalRead(BUTTON_UP_PIN);
  if (upButtonState == LOW || OTAEnabled == true)
  {
    setCpuFrequencyMhz(240); // stable 160,80,240
    Serial.println("button is pressed, enabling OTA");
    OTAEnabled = true;
    createWifiTask();
    display.clearDisplay();
    centerText("Connecting To WiFi", 30);
    centerText("Starting OTA", 40);
    manager.oledDisplay();
    manager.oledFadeOut();
    // Replace "1234" with a secure OTA password
    while (WiFi.status() != WL_CONNECTED)
    {
      delay(100);
    }
    Serial.println("Enabled OTA");
    ArduinoOTA.begin();
    display.clearDisplay();
    centerText("OTA:Enabled!", 10);
    centerText("IP Address:", 20);
    centerText(WiFi.localIP().toString(), 30);
    manager.oledDisplay();
    String ipAddress = WiFi.localIP().toString();

    // Extract the last two digits from the IP address
    String lastTwoDigits = ipAddress.substring(ipAddress.length() - 2);

    // Convert the last two digits string to an integer
    int lastTwoDigitsInt = lastTwoDigits.toInt();

    // Display the last two digits on the TM1637 display
    LedDisplay.showNumberDec(lastTwoDigitsInt);

    saveOtaValue(false);

    while (true)
    {
      display.clearDisplay();
      centerText("OTA:Enabled!", 10);
      centerText("IP Address:", 20);
      centerText(WiFi.localIP().toString(), 30);
      centerText("Running", 40);
      manager.oledDisplay();
      ArduinoOTA.handle();
      display.clearDisplay();
      centerText("OTA:Enabled!", 10);
      centerText("IP Address:", 20);
      centerText(WiFi.localIP().toString(), 30);
      manager.oledDisplay();
    }
    manager.oledDisable();
  }

  // Proceed with regular initialization if OTA is not enabled
  if (!OTAEnabled)
  {
    initialzeAlarmArray();
    readAlarms();
    createLedDisplayTask();
    createDimmingTask();
    createBatteryTask();
    createTempTask();
    createTimeTask();
    display.setFont(&DejaVu_LGC_Sans_Bold_10);
    setupScreensaver();
    createAlarmTask();
  }
  initMenus();
  delay(100);
}


void loop()
{
  Serial.println("TaskRunning");
  vTaskDelay(100);
}
