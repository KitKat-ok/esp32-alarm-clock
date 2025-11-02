#include "display.h"

SPIClass spi(FSPI); // ESP32-C6 uses FSPI as its main hardware SPI bus
Adafruit_SSD1327 oled(128, 128, &spi, OLED_DC, OLED_RESET, OLED_CS);

OLED_MANAGER oledMana;

void OLED_MANAGER::initDisplay()
{
    Serial.println("SSD1327 OLED init");
    spi.begin(OLED_CLK, -1, OLED_MOSI, OLED_CS); // use FSPI with custom pins
    if (!oled.begin(0x3D))
    {
        Serial.println("Unable to initialize OLED");
        while (1)
            yield();
    }
    oled.display();
}

void OLED_MANAGER::display()
{
    oled.display();
}

void OLED_MANAGER::disable()
{
    oledMana.ScreenEnabled = false;
    oled.oled_command(SSD1327_DISPLAYOFF);
}

void OLED_MANAGER::enable()
{
    oledMana.ScreenEnabled = true;
    oled.oled_command(SSD1327_DISPLAYON);
}

void OLED_MANAGER::fadeIn()
{
}

void OLED_MANAGER::fadeOut()
{
}
