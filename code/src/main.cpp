#include <Arduino.h>
#include <Adafruit_SSD1327.h>

#include <AS1115.h>

#include <Wire.h>
#include <AS1115.h>

AS1115 as = AS1115(0x00);

byte counter = 0;

// Used for software SPI
#define OLED_CLK 20
#define OLED_MOSI 19

// Used for software or hardware SPI
#define OLED_CS 22
#define OLED_DC 21

// Used for I2C or SPI
#define OLED_RESET -1

// software SPI
Adafruit_SSD1327 display(128, 128, OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);
// hardware SPI
// Adafruit_SSD1327 display(128, 128, &SPI, OLED_DC, OLED_RESET, OLED_CS);

// I2C
// Adafruit_SSD1327 display(128, 128, &Wire, OLED_RESET, 1000000);

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2

#define LOGO16_GLCD_HEIGHT 16
#define LOGO16_GLCD_WIDTH 16
static const unsigned char PROGMEM logo16_glcd_bmp[] =
    {B00000000, B11000000,
     B00000001, B11000000,
     B00000001, B11000000,
     B00000011, B11100000,
     B11110011, B11100000,
     B11111110, B11111000,
     B01111110, B11111111,
     B00110011, B10011111,
     B00011111, B11111100,
     B00001101, B01110000,
     B00011011, B10100000,
     B00111111, B11100000,
     B00111111, B11110000,
     B01111100, B11110000,
     B01110000, B01110000,
     B00000000, B00110000};

void testdrawbitmap(const uint8_t *bitmap, uint8_t w, uint8_t h)
{
  uint8_t icons[NUMFLAKES][3];
  randomSeed(666); // whatever seed

  // initialize
  for (uint8_t f = 0; f < NUMFLAKES; f++)
  {
    icons[f][XPOS] = random(display.width());
    icons[f][YPOS] = 0;
    icons[f][DELTAY] = random(5) + 1;

    Serial.print("x: ");
    Serial.print(icons[f][XPOS], DEC);
    Serial.print(" y: ");
    Serial.print(icons[f][YPOS], DEC);
    Serial.print(" dy: ");
    Serial.println(icons[f][DELTAY], DEC);
  }

  while (1)
  {
    // draw each icon
    for (uint8_t f = 0; f < NUMFLAKES; f++)
    {
      display.drawBitmap(icons[f][XPOS], icons[f][YPOS], bitmap, w, h, f + 1);
    }
    display.display();

    // then erase it + move it
    for (uint8_t f = 0; f < NUMFLAKES; f++)
    {
      display.drawBitmap(icons[f][XPOS], icons[f][YPOS], bitmap, w, h, SSD1327_BLACK);
      // move it
      icons[f][YPOS] += icons[f][DELTAY];
      // if its gone, reinit
      if (icons[f][YPOS] > display.height())
      {
        icons[f][XPOS] = random(display.width());
        icons[f][YPOS] = 0;
        icons[f][DELTAY] = random(5) + 1;
      }
    }
  }
}

void testdrawchar(void)
{
  display.setTextSize(1);
  display.setTextWrap(false);
  display.setTextColor(SSD1327_WHITE);
  display.setCursor(0, 0);

  for (uint8_t i = 0; i < 168; i++)
  {
    if (i == '\n')
      continue;
    display.write(i);
    if ((i > 0) && (i % 21 == 0))
      display.println();
  }
  display.display();
}

void testdrawcircle(void)
{
  for (uint8_t i = 0; i < display.height(); i += 2)
  {
    display.drawCircle(display.width() / 2, display.height() / 2, i, i % 15 + 1);
    display.display();
  }
}

void testfillrect(void)
{
  uint8_t color = 1;
  for (uint8_t i = 0; i < display.height() / 2; i += 3)
  {
    // alternate colors
    display.fillRect(i, i, display.width() - i * 2, display.height() - i * 2, i % 15 + 1);
    display.display();
    color++;
  }
}

void testdrawtriangle(void)
{
  for (uint16_t i = 0; i < min(display.width(), display.height()) / 2; i += 5)
  {
    display.drawTriangle(display.width() / 2, display.height() / 2 - i,
                         display.width() / 2 - i, display.height() / 2 + i,
                         display.width() / 2 + i, display.height() / 2 + i, i % 15 + 1);
    display.display();
  }
}

void testfilltriangle(void)
{
  // uint8_t color = SSD1327_WHITE;
  for (int16_t i = min(display.width(), display.height()) / 2; i > 0; i -= 5)
  {
    display.fillTriangle(display.width() / 2, display.height() / 2 - i,
                         display.width() / 2 - i, display.height() / 2 + i,
                         display.width() / 2 + i, display.height() / 2 + i, i % 15 + 1);
    display.display();
  }
}

void testdrawroundrect(void)
{
  for (uint8_t i = 0; i < display.height() / 3 - 2; i += 2)
  {
    display.drawRoundRect(i, i, display.width() - 2 * i, display.height() - 2 * i, display.height() / 4, i % 15 + 1);
    display.display();
  }
}

void testfillroundrect(void)
{
  // uint8_t color = SSD1327_WHITE;
  for (uint8_t i = 0; i < display.height() / 3 - 2; i += 2)
  {
    display.fillRoundRect(i, i, display.width() - 2 * i, display.height() - 2 * i, display.height() / 4, i % 15 + 1);
    display.display();
  }
}

void testdrawrect(void)
{
  for (uint8_t i = 0; i < display.height() / 2; i += 2)
  {
    display.drawRect(i, i, display.width() - 2 * i, display.height() - 2 * i, i % 15 + 1);
    display.display();
  }
}

void testdrawline()
{
  for (uint8_t i = 0; i < display.width(); i += 4)
  {
    display.drawLine(0, 0, i, display.height() - 1, SSD1327_WHITE);
    display.display();
  }
  for (uint8_t i = 0; i < display.height(); i += 4)
  {
    display.drawLine(0, 0, display.width() - 1, i, SSD1327_WHITE);
    display.display();
  }
  delay(250);

  display.clearDisplay();
  for (uint8_t i = 0; i < display.width(); i += 4)
  {
    display.drawLine(0, display.height() - 1, i, 0, SSD1327_WHITE);
    display.display();
  }
  for (int8_t i = display.height() - 1; i >= 0; i -= 4)
  {
    display.drawLine(0, display.height() - 1, display.width() - 1, i, SSD1327_WHITE);
    display.display();
  }
  delay(250);

  display.clearDisplay();
  for (int8_t i = display.width() - 1; i >= 0; i -= 4)
  {
    display.drawLine(display.width() - 1, display.height() - 1, i, 0, SSD1327_WHITE);
    display.display();
  }
  for (int8_t i = display.height() - 1; i >= 0; i -= 4)
  {
    display.drawLine(display.width() - 1, display.height() - 1, 0, i, SSD1327_WHITE);
    display.display();
  }
  delay(250);

  display.clearDisplay();
  for (uint8_t i = 0; i < display.height(); i += 4)
  {
    display.drawLine(display.width() - 1, 0, 0, i, SSD1327_WHITE);
    display.display();
  }
  for (uint8_t i = 0; i < display.width(); i += 4)
  {
    display.drawLine(display.width() - 1, 0, i, display.height() - 1, SSD1327_WHITE);
    display.display();
  }
  delay(250);
}

void setup()
{
  Serial.begin(9600);
  // while (! Serial) delay(100);
  Serial.println("SSD1327 OLED test");

  if (!display.begin(0x3D))
  {
    Serial.println("Unable to initialize OLED");
    while (1)
      yield();
  }
  display.clearDisplay();
  display.display();
  display.invertDisplay(true);
  delay(1000);
  Serial.println("done.");
  Wire.begin(6, 7, 100000); // 100 kHz
  as.init(4, 6);
  as.clear();
  as.display("88.88");
  as.setIntensity(0);
}

void loop()
{
}