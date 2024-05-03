#include "defines.h"
#include <ArduinoOTA.h>

bool OTAEnabled = false;

void setup()
{
  Serial.println("Initializing Hardware");

  initHardware();
  initWifi();

  // Check if both buttons are pressed to enable OTA
  int confirmButtonState = digitalRead(BUTTON_CONFIRM_PIN);
  int exitButtonState = digitalRead(BUTTON_EXIT_PIN);
  if (confirmButtonState == LOW && exitButtonState == LOW)
  {
    Serial.println("Both buttons are pressed, enabling OTA");
    OTAEnabled = true;
    createWifiTask();
    display.clearDisplay();
    centerText("Connecting To WiFi", 30);
    centerText("Starting OTA", 40);
    display.display();
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
    display.display();
    String ipAddress = WiFi.localIP().toString();

    // Extract the last two digits from the IP address
    String lastTwoDigits = ipAddress.substring(ipAddress.length() - 2);

    // Convert the last two digits string to an integer
    int lastTwoDigitsInt = lastTwoDigits.toInt();

    // Display the last two digits on the TM1637 display
    LedDisplay.showNumberDec(lastTwoDigitsInt);

    while (true)
    {
      ArduinoOTA.handle();
    }
  }

  // Proceed with regular initialization if OTA is not enabled
  if (!OTAEnabled)
  {
    initBattery();
    createDimmingTask();
    createBatteryTask();
    initMenus();
    readAlarms();
    createTempTask();
    createLedDisplayTask();
    createTimeTask();
    display.setFont(&DejaVu_LGC_Sans_Bold_10);
  }
}

bool initialMenuRunning = false;

void loop()
{
  if (menuRunning != initialMenuRunning)
  {
    Serial.println("Menu state has changed.");
    display.clearDisplay();
    display.display();
    delay(500);
    display.clearDisplay();
    display.display();
    wakeUpMenu();
    menuRunning = false;
    initialMenuRunning = menuRunning;
  }
  if (menuRunning == false)
  {
    handleMenus();
    display.display();
  }
}
