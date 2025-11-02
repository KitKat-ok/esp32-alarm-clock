#include "ota.h"

void checkForRunOta()
{
    if (buttonRead(UP_PIN) == HIGH || readOtaValue() == true)
    {
        setCpuFrequencyMhz(240); // stable 160,80,240
        Serial.println("button is pressed or ota enabled from last boot");
        createWifiTask();
        oled.clearDisplay();
        centerText("Connecting To WiFi", 30);
        centerText("Starting OTA", 40);
        oled.display();
        while (WiFi.status() != WL_CONNECTED)
        {
            delay(100);
        }
        Serial.println("Enabled OTA");
        ArduinoOTA.begin();
        String ipAddress = WiFi.localIP().toString();

        String lastTwoDigits = ipAddress.substring(ipAddress.length() - 2);

        int lastTwoDigitsInt = lastTwoDigits.toInt();

        LedDisplay.display(lastTwoDigitsInt);

        saveOtaValue(false);

        while (true)
        {
            oled.clearDisplay();
            centerText("OTA:Enabled!", 10);
            centerText("IP Address:", 20);
            centerText(WiFi.localIP().toString(), 30);
            centerText("Running", 40);
            oledMana.display();
            ArduinoOTA.handle();
            oled.clearDisplay();
            centerText("OTA:Enabled!", 10);
            centerText("IP Address:", 20);
            centerText(WiFi.localIP().toString(), 30);
            oledMana.display();
        }
        oledMana.disable();
    }
}

void startOTA()
{
    saveOtaValue(true);
    ESP.restart();
}