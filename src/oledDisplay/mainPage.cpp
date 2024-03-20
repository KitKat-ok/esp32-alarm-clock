#include "mainPage.h"

static unsigned long lastExecutionTime = 0;

void showMainPage()
{
    unsigned long currentTime = millis();       // Get the current time

    // Check if 15 seconds have passed since the last execution
    if (currentTime - lastExecutionTime >= 2000)
    {
        // Update the last execution time
        lastExecutionTime = currentTime;

        // Your existing code for showMainPage() here
        display.clearDisplay();
        display.drawRect(0, SCREEN_HEIGHT / 3 - 5, SCREEN_WIDTH, 2, SSD1306_WHITE);
        display.setTextSize(1);
        display.setFont(&DejaVu_LGC_Sans_15);
        centerText(String(day()) + "/" + String(month()) + "/" + String(year()), SCREEN_HEIGHT / 2);
        display.setTextSize(1);
        display.setFont(&DejaVu_LGC_Sans_Bold_10);
        if (charging == true)
        {
            centerText(String(batteryPercentage) + "%", SCREEN_HEIGHT / 3 - 7);
            display.fillCircle(42, (SCREEN_HEIGHT / 3 - 1) / 2, 2, SSD1306_WHITE);
        }
        else
        {
            centerText(String(batteryPercentage) + "%", SCREEN_HEIGHT / 3 - 7);
        }

        centerText(getCurrentWeekdayName(), SCREEN_HEIGHT / 2 + 10);
        centerText("Temp: " + String(temperature) + " C", SCREEN_HEIGHT / 2 + 20);
        display.display();
    }
}
