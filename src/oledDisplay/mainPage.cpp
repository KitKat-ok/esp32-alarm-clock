#include "mainPage.h"

void showMainPage()
{
    display.clearDisplay();
    display.drawRect(0, SCREEN_HEIGHT / 3 - 5, SCREEN_WIDTH, 2, SSD1306_WHITE);
    display.setTextSize(1);
    display.setFont(&DejaVu_LGC_Sans_15);
    centerText(String(day()) + "/" + String(month()) + "/" + String(year()), SCREEN_HEIGHT / 2);
    display.setTextSize(1);
    display.setFont(&DejaVu_LGC_Sans_Bold_10);
    centerText(String(batteryPercentage) + "%", SCREEN_HEIGHT / 3 - 7);
    centerText(getCurrentWeekdayName(), SCREEN_HEIGHT / 2 + 10);
    centerText("Temp: " + String(temperature) + " C", SCREEN_HEIGHT / 2 + 20);
    display.display();
}
