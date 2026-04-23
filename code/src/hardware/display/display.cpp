#include "display.h"

SPIClass spi(FSPI); // ESP32-C6 uses FSPI as its main hardware SPI bus
Adafruit_SSD1327 oled(128, 128, &spi, OLED_DC, OLED_RESET, OLED_CS, 10000000UL);

OLED_MANAGER oledMana;

#define MIN_CONTRAST 100
#define MAX_CONTRAST 255

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
    oled.oled_command(0x81);
    oled.oled_command(MAX_CONTRAST); // contrast

    oled.oled_command(0xB1);
    oled.oled_command(0xE2); // phase length

    oled.oled_command(0xB3);
    oled.oled_command(0xA1); // clock divider

    oled.oled_command(0xBC);
    oled.oled_command(0x01); // precharge 

    oled.oled_command(0xBE);
    oled.oled_command(0x10); // VCOMH voltage

    oled.oled_command(0xB6);
    oled.oled_command(0x01); // second precharge period

    oled.clearDisplay();

    int barWidth = oled.width() / 16;
    for (int i = 0; i < 16; i++)
    {
        uint16_t x = i * barWidth;
        oled.fillRect(x, 0, barWidth, oled.height(), i);
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
    if (dimmed == true)
    {
        for (int dim = MIN_CONTRAST; dim <= MAX_CONTRAST; dim += 10)
        {
            oled.oled_command(0x81);
            oled.oled_command(dim);
            delay(20);
        }
        dimmed = false;
    }
}

void OLED_MANAGER::fadeOut()
{
    if (dimmed == false)
    {
        for (int dim = MAX_CONTRAST; dim >= MIN_CONTRAST; dim -= 10)
        {
            oled.oled_command(0x81);
            oled.oled_command(dim);
            delay(20);
        }
        dimmed = true;
    }
}
