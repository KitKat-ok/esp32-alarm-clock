#include "mainPage.h"

static unsigned long lastExecutionTime = 0;

void showMainPage()
{
    unsigned long currentTime = millis(); // Get the current time

    // Check if 15 seconds have passed since the last execution
    if (currentTime - lastExecutionTime >= 2000)
    {
        // Update the last execution time
        lastExecutionTime = currentTime;

        // Your existing code for showMainPage() here
        display.clearDisplay();
        display.drawRect(0, SCREEN_HEIGHT / 3 - 5, SCREEN_WIDTH, 2, SSD1306_WHITE);
        display.setTextSize(1);
        display.setFont(&DejaVu_Sans_Bold_14);
        centerText(String(day()) + "." + String(month()) + "." + String(year()), SCREEN_HEIGHT / 2);
        display.setTextSize(1);
        display.setFont(&DejaVu_LGC_Sans_Bold_10);
        if (charging == true)
        {
            display.setCursor(SCREEN_WIDTH - 37, (SCREEN_HEIGHT / 3 - 5)-4);
            display.println(String(batteryPercentage) + "%");
            display.fillCircle((SCREEN_WIDTH - 42), (SCREEN_HEIGHT / 3 - 5) / 2, 2, SSD1306_WHITE);
        }
        else
        {
            display.setCursor(SCREEN_WIDTH - 37, (SCREEN_HEIGHT / 3 - 5)-4);
            display.println(String(batteryPercentage) + "%");
        }
        display.drawLine(SCREEN_WIDTH - 47, SCREEN_HEIGHT / 3 - 5, SCREEN_WIDTH - 47, 0, SSD1306_WHITE);
        centerText(getCurrentWeekdayName() + "/" + getCurrentMonthName(), SCREEN_HEIGHT / 2 + 10);
        centerText("Temp: " + String(temperature) + " C", SCREEN_HEIGHT / 2 + 23);
        display.drawLine(26 - 8, 45, 102 + 8, 45, WHITE);
  int bars = map(batteryPercentage, 0, 100, 0, 8); // Map battery percentage to bars
  for (int i = 0; i < bars; i++) {
    display.fillRect(i * 10, (SCREEN_HEIGHT / 3 - 12)-4, 8, 8, SSD1306_WHITE); // Draw bars
  }
        display.display();
    }
}
