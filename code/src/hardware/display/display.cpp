#include "display.h"

SPIClass spi(FSPI);
Adafruit_SSD1327 oled(128, 128, &spi, OLED_DC, OLED_RESET, OLED_CS, 5000000UL);

OLED_MANAGER oledMana;

#define MIN_CONTRAST 50
#define MAX_CONTRAST 255

const uint8_t graytable_highCotr[] = {
    SSD1327_GRAYTABLE,
    0x03, 0x05, 0x07, 0x09,
    0x0C, 0x0F, 0x12, 0x16,
    0x1A, 0x1F, 0x25, 0x2B,
    0x32, 0x38, 0x3C, 0x3F};

const uint8_t graytable_lowCotr[] = {
    SSD1327_GRAYTABLE,
    0x05, 0x07, 0x09, 0x0B,
    0x0E, 0x11, 0x14, 0x18,
    0x1D, 0x22, 0x28, 0x2E,
    0x34, 0x39, 0x3D, 0x3F};

SemaphoreHandle_t oledMutex = nullptr;

void OLED_MANAGER::initDisplay()
{
    oledMutex = xSemaphoreCreateMutex();
    Serial.println("SSD1327 OLED init");
    spi.begin(OLED_CLK, -1, OLED_MOSI, OLED_CS);
    if (!oled.begin(0x3D, true))
    {
        Serial.println("Unable to initialize OLED");
        while (1)
            yield();
    }
    xSemaphoreTake(oledMutex, portMAX_DELAY);

    const uint8_t init_128x128[] = {
        // Init sequence for 128x128 OLED module
        SSD1327_DISPLAYOFF, // 0xAE
        SSD1327_SETCONTRAST,
        0x80,             // 0x81
        SSD1327_SEGREMAP, // 0xA0
        0x51,             // remap memory, odd even columns, com flip and column swap
        SSD1327_SETSTARTLINE,
        0x00, // 0xA1
        SSD1327_SETDISPLAYOFFSET,
        0x00, // 0xA2
        SSD1327_DISPLAYALLOFF, SSD1327_SETMULTIPLEX,
        0x7F,
        SSD1327_PHASELEN,
        0x51, // 0xB1
        SSD1327_GRAYTABLE,
        0x03, 0x05, 0x07, 0x09,
        0x0C, 0x0F, 0x12, 0x16,
        0x1A, 0x1F, 0x25, 0x2B,
        0x32, 0x38, 0x3C, 0x3F,
        SSD1327_DCLK,
        0x00, // 0xb3
        SSD1327_REGULATOR,
        0x01, // 0xAB
        SSD1327_PRECHARGE2,
        0x04, // 0xB6
        SSD1327_SETVCOM,
        0x0F, // 0xBE
        SSD1327_PRECHARGE,
        0x22, // 0xBC
        SSD1327_FUNCSELB,
        0x62, // 0xD5
        SSD1327_CMDLOCK,
        0x12, // 0xFD
        SSD1327_NORMALDISPLAY, SSD1327_DISPLAYON};

    if (!oled.oled_commandList(init_128x128, sizeof(init_128x128)))
    {
        return;
    }
    xSemaphoreGive(oledMutex);

    oled.clearDisplay();
    oled.display();

    // int barWidth = oled.width() / 16;

    // for (int i = 0; i < 16; i++)
    // {
    //     uint16_t x = i * barWidth;
    //     oled.fillRect(x, 0, barWidth, oled.height(), i);
    // }
    // OLED_MANAGER::fadeIn();

    // oled.display();

    // delay(1000);

    // OLED_MANAGER::fadeOut();

    // delay(1000);
}

void OLED_MANAGER::display()
{
    if (xSemaphoreTake(oledMutex, portMAX_DELAY))
    {
        oled.display();
        xSemaphoreGive(oledMutex);
    }
}

void OLED_MANAGER::disable()
{
    if (xSemaphoreTake(oledMutex, portMAX_DELAY))
    {
        ScreenEnabled = false;
        oled.oled_command(SSD1327_DISPLAYOFF);
        xSemaphoreGive(oledMutex);
    }
}

void OLED_MANAGER::enable()
{
    if (xSemaphoreTake(oledMutex, portMAX_DELAY))
    {
        ScreenEnabled = true;
        oled.oled_command(SSD1327_DISPLAYON);
        xSemaphoreGive(oledMutex);
    }
}

void OLED_MANAGER::fadeIn()
{
    if (!dimmed)
        return;

    if (xSemaphoreTake(oledMutex, portMAX_DELAY))
    {
        for (int dim = MIN_CONTRAST; dim <= MAX_CONTRAST; dim += 10)
        {
            oled.oled_command(0x81);
            oled.oled_command(dim);
            delay(20);
        }

        if (!oled.oled_commandList(graytable_highCotr, sizeof(graytable_highCotr)))
        {
            return;
        }

        dimmed = false;

        xSemaphoreGive(oledMutex);
    }
}

void OLED_MANAGER::fadeOut()
{
    if (dimmed)
        return;

    if (xSemaphoreTake(oledMutex, portMAX_DELAY))
    {
        for (int dim = MAX_CONTRAST; dim >= MIN_CONTRAST; dim -= 10)
        {
            oled.oled_command(0x81);
            oled.oled_command(dim);
            delay(20);
        }

        if (!oled.oled_commandList(graytable_lowCotr, sizeof(graytable_lowCotr)))
        {
            return;
        }

        dimmed = true;

        xSemaphoreGive(oledMutex);
    }
}