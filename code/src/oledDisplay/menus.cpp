#include "menus.h"

bool displayed = false;

void initWeatherMenu()
{
    displayed = false;
}

void checkExit()
{
    static bool exitHeld = false;
    if (checkButtonReleased(BUTTON_EXIT_PIN, exitHeld))
    {
        manager.stopScrolling();
        displayed = false;
        exitLoopFunction = true;
    }
}

void currentWeatherMenu()
{
    currentWeather();
    checkExit();
}

void currentWeather()
{
    display.clearDisplay(); // Clear the display

    // Use the weather condition ID from CurrentWeatherData to select the appropriate widget
    displaywidget(currentWeatherData.weatherConditionId);

    // Set text size and font
    display.setTextSize(1);
    display.setFont(&DejaVu_LGC_Sans_Bold_10);

    // Display the current day of the week
    display.setCursor(54, 20);
    display.print(getCurrentWeekdayName());

    // Draw a separator line
    display.drawLine(58, 25, 127, 25, WHITE);

    // Display the current temperature
    display.setCursor(58, 40);
    display.setFont(&DejaVu_Sans_Bold_16);
    display.print(String(currentWeatherData.temp, 1) + "C"); // Show temperature with 1 decimal place

    // Display wind speed and direction
    display.setFont(&DejaVu_LGC_Sans_Bold_10);
    display.setCursor(58, 48);
    display.print(currentWeatherData.windSpeed, 1); // Wind speed with 1 decimal place
    display.print(" m/s ");
    display.print(convertWindDirection(currentWeatherData.windDirection));

    // Display the weather condition description
    display.setCursor(0, SCREEN_HEIGHT - 5);
    display.setFont(&Roboto_Black_9);
    display.fillRect(0, SCREEN_HEIGHT - 16, SCREEN_WIDTH, 16, SSD1306_BLACK);
    display.print(currentWeatherData.main); // Print the weather condition description
    display.print("\t");
    display.print("");

    // Scroll the display content
    display.setCursor(54, 10);
    display.setFont(&DejaVu_LGC_Sans_Bold_10);

    display.println(String(day()) + "." + String(month()) + "." + String(year()));
    delay(10);
    manager.oledDisplay();
    manager.startScrollingLeft(0x06, 0x07, 2);

    // Restore the font settings
    display.setFont(&DejaVu_LGC_Sans_Bold_10);
}

void displayWeatherCast(int dayIndex)
{
    checkExit();
    if (displayed == false)
    {
        displayed = true;
        display.clearDisplay();
        display.setTextColor(SSD1306_WHITE);
        displaywidget(weatherDailyForecastData[dayIndex].weatherConditionId);
        display.setTextSize(1);
        display.setCursor(55, 10);
        display.setFont(&DejaVu_LGC_Sans_Bold_9);
        display.print(getNextDayName(dayIndex));
        display.drawLine(57, 15, 127, 15, WHITE);
        display.setFont(&DejaVu_LGC_Sans_Bold_10);

        float tmax = weatherDailyForecastData[dayIndex].maxTemp;
        float tmin = weatherDailyForecastData[dayIndex].minTemp;

        display.setCursor(57, 25);
        display.print("Min " + String(tmin));
        display.setCursor(57, 35);
        display.print("Max " + String(tmax));

        display.setCursor(57, 45);
        display.print(weatherDailyForecastData[dayIndex].windSpeed);
        display.print("m/s ");
        display.println(convertWindDirection(weatherDailyForecastData[dayIndex].windDirection));

        display.setCursor(0, SCREEN_HEIGHT - 5);
        display.setFont(&DejaVu_LGC_Sans_Bold_9);
        display.fillRect(0, SCREEN_HEIGHT - 16, SCREEN_WIDTH, 16, SSD1306_BLACK);

        if (isWeatherAvailable == true)
        {
            display.setCursor(0, SCREEN_HEIGHT - 5);
            display.print(weatherConditionIdToStr(weatherDailyForecastData[dayIndex].weatherConditionId));
            display.print("");
        }
        else
        {
            display.setCursor(0, SCREEN_HEIGHT - 5);
            display.print("N/A                     ");
            display.print("");
        }

        manager.oledDisplay();
        manager.startScrollingLeft(0x06, 0x07, 2);
        display.setFont(&DejaVu_LGC_Sans_Bold_10);
    }
}

String convertWindDirection(uint16_t degrees)
{
    if (degrees >= 337.5 || degrees < 22.5)
    {
        return "N";
    }
    else if (degrees < 112.5)
    {
        return "E";
    }
    else if (degrees < 202.5)
    {
        return "S";
    }
    else if (degrees < 292.5)
    {
        return "W";
    }
    else
    {
        return "-";
    }
}
void fpsCalc()
{
    unsigned long startTime, endTime;
    int numFrames = 100; // Number of frames to measure

    // Measure FPS for drawing lines
    startTime = millis();
    for (int i = 0; i < numFrames; i++)
    {
        display.clearDisplay();
        for (int x = 0; x < SCREEN_WIDTH; x += 2)
        {
            display.drawLine(x, 0, SCREEN_WIDTH - x, SCREEN_HEIGHT - 1, SSD1306_WHITE); // Draw line
        }
        for (int y = 0; y < SCREEN_HEIGHT; y += 2)
        {
            display.drawLine(0, y, SCREEN_WIDTH - 1, SCREEN_HEIGHT - y, SSD1306_WHITE); // Draw line
        }
        manager.oledDisplay(); // Refresh display
    }
    endTime = millis();
    float elapsedTime = (endTime - startTime) / 1000.0; // Convert to seconds
    float fps = numFrames / elapsedTime;

    // Display FPS for line drawing on OLED
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.print("Line Drawing FPS: ");
    display.print(fps, 1); // Display FPS with one decimal place
    manager.oledDisplay();
    delay(5000); // Show FPS for 5 seconds

    // Measure FPS for drawing text
    startTime = millis();
    for (int i = 0; i < numFrames; i++)
    {
        display.clearDisplay();
        for (int y = 0; y < SCREEN_HEIGHT; y += 8)
        {
            display.setCursor(0, y);
            display.setTextSize(1);
            display.setTextColor(SSD1306_WHITE);
            display.print("FPS Test");
        }
        manager.oledDisplay(); // Refresh display
    }
    endTime = millis();
    elapsedTime = (endTime - startTime) / 1000.0; // Convert to seconds
    fps = numFrames / elapsedTime;

    // Display FPS for text drawing on OLED
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.print("Text Drawing FPS: ");
    display.print(fps, 1); // Display FPS with one decimal place
    manager.oledDisplay();
    delay(5000); // Show FPS for 5 seconds
}

void wifiDebugMenu()
{
    checkExit();
    display.clearDisplay();
    centerText("WiFi debug", 10);
    display.drawRect(0, SCREEN_HEIGHT / 3 - 8, SCREEN_WIDTH, 2, SSD1306_WHITE);
    display.setFont(&DejaVu_LGC_Sans_Bold_9);
    display.setCursor(0, 24);
    display.println("WiFi SSID: " + String(WiFi.SSID()));
    centerText("IP address:", 33);
    centerText(WiFi.localIP().toString(), 43);
    centerText("Mac address: ", 53);
    centerText(String(WiFi.macAddress()), 63);
    display.setFont(&DejaVu_LGC_Sans_Bold_10);
    manager.oledDisplay();
}

void CPUDebugMenu()
{
    checkExit();
    display.clearDisplay();
    centerText("CPU debug", 10);
    display.drawRect(0, SCREEN_HEIGHT / 3 - 8, SCREEN_WIDTH, 2, SSD1306_WHITE);
    display.setFont(&DejaVu_LGC_Sans_Bold_9);
    display.setCursor(0, 24);
    display.println("CPU freq: " + String(getCpuFrequencyMhz()) + " Mhz");
    display.setCursor(0, 34);
    display.println("XTAL freq: " + String(getXtalFrequencyMhz()) + " Mhz");
    centerText("Chip model:", 44);
    display.setFont(&DejaVu_LGC_Sans_Bold_10);
    manager.oledDisplay();
}

void generalDebugMenu()
{
    checkExit();
    display.clearDisplay();
    centerText("General debug ", 10);
    display.drawRect(0, SCREEN_HEIGHT / 3 - 8, SCREEN_WIDTH, 2, SSD1306_WHITE);
    display.setFont(&DejaVu_LGC_Sans_Bold_9);
    centerText("Reset reason:", 23);
    centerText(resetReasonToString(esp_reset_reason()), 33);
    display.setCursor(0, 43);
    display.println("Uptime: " + microsecondsToTimeString(esp_timer_get_time()) + " " + touchRead(TOUCH_BUTTON_PIN) + " " + smoothTouchRead(TOUCH_BUTTON_PIN));
    display.setCursor(0, 53);
    display.println("Free heap: " + String(esp_get_free_heap_size() / 1024.0, 2) + " KB");
    display.setCursor(0, 63);
    display.println("Battery Vol: " + String(getBatteryVoltage()));
    if (charging == true)
    {
        display.fillCircle((SCREEN_WIDTH - 20), 60, 3, SSD1306_WHITE);
    }
    display.setFont(&DejaVu_LGC_Sans_Bold_10);
    manager.oledDisplay();
}
