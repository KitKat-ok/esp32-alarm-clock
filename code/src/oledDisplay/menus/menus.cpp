#include "menus.h"

bool displayed = false;

void initWeatherMenu()
{
    displayed = false;
}

void checkExit()
{

    if (shouldExitLoop() == true)
    {
        //oledMana.sendOledAction(OLED_STOP_SCROLL);
        displayed = false;
        exitLoopFunction = true;
    }
}

void currentWeatherMenu()
{
    if (displayed == false)
    {
        currentWeather();
        displayed = true;
    }
    checkExit();
}

void currentWeather()
{
oled.clearDisplay(); // Clear the display

    // Use the weather condition ID from CurrentWeatherData to select the appropriate widget
    displaywidget(currentWeatherData.weatherConditionId);

    // Set text size and font
oled.setTextSize(1);
oled.setFont(&DejaVu_LGC_Sans_Bold_10);

    // Display the current day of the week
oled.setCursor(54, 20);
oled.print(getCurrentWeekdayName());

    // Draw a separator line
oled.drawLine(58, 25, 127, 25, SSD1327_WHITE);

    // Display the current temperature
oled.setCursor(58, 40);
oled.setFont(&DejaVu_Sans_Bold_16);
oled.print(String(currentWeatherData.temp, 1) + "C"); // Show temperature with 1 decimal place

    // Display wind speed and direction
oled.setFont(&DejaVu_LGC_Sans_Bold_10);
oled.setCursor(58, 48);
oled.print(currentWeatherData.windSpeed, 1); // Wind speed with 1 decimal place
oled.print(" m/s ");
oled.print(convertWindDirection(currentWeatherData.windDirection));
    // Display the weather condition description
oled.setCursor(1, SCREEN_HEIGHT - 5);
oled.setFont(&Roboto_Black_9);
oled.fillRect(0, SCREEN_HEIGHT - 16, SCREEN_WIDTH, 16, SSD1327_BLACK);
    if (isWeatherAvailable == true)
    {
oled.print(" " + currentWeatherData.main); // Print the weather condition description
    }
    else
    {
oled.setCursor(0, SCREEN_HEIGHT - 5);
oled.print("N/A                     ");
    }
oled.print("\t");
oled.print("");

    // Scroll the display content
oled.setCursor(54, 10);
oled.setFont(&DejaVu_LGC_Sans_Bold_10);

oled.println(String(day()) + "." + String(month()) + "." + String(year()));
    delay(10);
    oledMana.display();
    //oledMana.sendOledAction(OLED_SCROLL_LEFT, 0x06, 0x07, 2);

    // Restore the font settings
oled.setFont(&DejaVu_LGC_Sans_Bold_10);
}

void displayWeatherCast(int dayIndex)
{
    checkExit();
    if (displayed == false)
    {
        displayed = true;
oled.clearDisplay();
oled.setTextColor(SSD1327_WHITE);
        displaywidget(weatherDailyForecastData[dayIndex].weatherConditionId);
oled.setTextSize(1);
oled.setCursor(55, 10);
oled.setFont(&DejaVu_LGC_Sans_Bold_9);
oled.print(getNextDayName(dayIndex));
oled.drawLine(57, 15, 127, 15, SSD1327_WHITE);
oled.setFont(&DejaVu_LGC_Sans_Bold_10);

        float tmax = weatherDailyForecastData[dayIndex].maxTemp;
        float tmin = weatherDailyForecastData[dayIndex].minTemp;

oled.setCursor(57, 25);
oled.print("Min " + String(tmin));
oled.setCursor(57, 35);
oled.print("Max " + String(tmax));

oled.setCursor(57, 45);
oled.print(weatherDailyForecastData[dayIndex].windSpeed);
oled.print("m/s ");
oled.println(convertWindDirection(weatherDailyForecastData[dayIndex].windDirection));

oled.setCursor(0, SCREEN_HEIGHT - 5);
oled.setFont(&DejaVu_LGC_Sans_Bold_9);
oled.fillRect(0, SCREEN_HEIGHT - 16, SCREEN_WIDTH, 16, SSD1327_BLACK);

        if (isWeatherAvailable == true)
        {
oled.setCursor(1, SCREEN_HEIGHT - 5);
oled.print(weatherConditionIdToStr(weatherDailyForecastData[dayIndex].weatherConditionId));
oled.print("");
        }
        else
        {
oled.setCursor(0, SCREEN_HEIGHT - 5);
oled.print("N/A                     ");
oled.print("");
        }

        oledMana.display();
        //oledMana.sendOledAction(OLED_SCROLL_LEFT, 0x06, 0x07, 2);
oled.setFont(&DejaVu_LGC_Sans_Bold_10);
    }
}

String convertWindDirection(uint16_t degrees)
{
    Serial.println("Current wind direction" + String(degrees));
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
    checkExit();
    unsigned long startTime, endTime;
    int numFrames = 100; // Number of frames to measure

    // Measure FPS for drawing lines
    startTime = millis();
    for (int i = 0; i < numFrames; i++)
    {
        checkExit();
oled.clearDisplay();
        for (int x = 0; x < SCREEN_WIDTH; x += 2)
        {
oled.drawLine(x, 0, SCREEN_WIDTH - x, SCREEN_HEIGHT - 1, SSD1327_WHITE); // Draw line
        }
        for (int y = 0; y < SCREEN_HEIGHT; y += 2)
        {
oled.drawLine(0, y, SCREEN_WIDTH - 1, SCREEN_HEIGHT - y, SSD1327_WHITE); // Draw line
        }
        oledMana.display(); // Refresh display
    }
    endTime = millis();
    float elapsedTime = (endTime - startTime) / 1000.0; // Convert to seconds
    float fps = numFrames / elapsedTime;

    // Display FPS for line drawing on OLED
oled.clearDisplay();
oled.setTextSize(1);
oled.setTextColor(SSD1327_WHITE);
oled.setCursor(0, 0);
oled.print("Line Drawing FPS: ");
oled.print(fps, 1); // Display FPS with one decimal place
    oledMana.display();
    delay(5000); // Show FPS for 5 seconds

    // Measure FPS for drawing text
    startTime = millis();
    for (int i = 0; i < numFrames; i++)
    {
oled.clearDisplay();
        for (int y = 0; y < SCREEN_HEIGHT; y += 8)
        {
oled.setCursor(0, y);
oled.setTextSize(1);
oled.setTextColor(SSD1327_WHITE);
oled.print("FPS Test");
        }
        oledMana.display(); // Refresh display
    }
    endTime = millis();
    elapsedTime = (endTime - startTime) / 1000.0; // Convert to seconds
    fps = numFrames / elapsedTime;

    // Display FPS for text drawing on OLED
oled.clearDisplay();
oled.setTextSize(1);
oled.setTextColor(SSD1327_WHITE);
oled.setCursor(0, 0);
oled.print("Text Drawing FPS: ");
oled.print(fps, 1); // Display FPS with one decimal place
    oledMana.display();
    delay(5000); // Show FPS for 5 seconds
}

void wifiDebugMenu()
{
    checkExit();
oled.clearDisplay();
    centerText("WiFi debug", 10);
oled.drawRect(0, SCREEN_HEIGHT / 3 - 8, SCREEN_WIDTH, 2, SSD1327_WHITE);
oled.setFont(&DejaVu_LGC_Sans_Bold_9);
oled.setCursor(0, 24);
oled.println("WiFi SSID: " + String(WiFi.SSID()));
    centerText("IP address:", 33);
    centerText(WiFi.localIP().toString(), 43);
    centerText("Mac address: ", 53);
    centerText(String(WiFi.macAddress()), 63);
oled.setFont(&DejaVu_LGC_Sans_Bold_10);
    oledMana.display();
    delay(10);
}

void CPUDebugMenu()
{
    checkExit();
oled.clearDisplay();
    centerText("CPU debug", 10);
oled.drawRect(0, SCREEN_HEIGHT / 3 - 8, SCREEN_WIDTH, 2, SSD1327_WHITE);
oled.setFont(&DejaVu_LGC_Sans_Bold_9);
oled.setCursor(0, 24);
oled.println("CPU freq: " + String(getCpuFrequencyMhz()) + " Mhz");
oled.setCursor(0, 34);
oled.println("XTAL freq: " + String(getXtalFrequencyMhz()) + " Mhz");
    centerText("Chip model:", 44);
    centerText(String(ESP.getChipModel()), 54);
oled.setFont(&DejaVu_LGC_Sans_Bold_10);
    oledMana.display();
    delay(10);
}

void generalDebugMenu()
{
    checkExit();
oled.clearDisplay();
    centerText("General debug ", 10);
oled.drawRect(0, SCREEN_HEIGHT / 3 - 8, SCREEN_WIDTH, 2, SSD1327_WHITE);
oled.setFont(&DejaVu_LGC_Sans_Bold_9);
    centerText("Reset reason:", 23);
    centerText(resetReasonToString(esp_reset_reason()), 33);
oled.setCursor(0, 43);
oled.println("Uptime: " + microsecondsToTimeString(esp_timer_get_time()));
oled.setCursor(0, 53);
oled.println("Free heap: " + String(esp_get_free_heap_size() / 1024.0, 2) + " KB");
oled.setCursor(0, 63);
oled.println("Battery Vol: " + String(getBatteryVoltage()));
    if (charging == true)
    {
oled.fillCircle((SCREEN_WIDTH - 20), 60, 3, SSD1327_WHITE);
    }
oled.setFont(&DejaVu_LGC_Sans_Bold_10);
    oledMana.display();
    delay(10);
}

void touchDebugMenu()
{
    Serial.println("checking Exit");
    checkExit();
    Serial.println(" finished checking Exit");
oled.clearDisplay();
    centerText("Touch Debug ", 10);
oled.drawRect(0, SCREEN_HEIGHT / 3 - 8, SCREEN_WIDTH, 2, SSD1327_WHITE);
oled.setFont(&DejaVu_LGC_Sans_Bold_9);
oled.setCursor(0, 23);
    //display.println("First Seg " + String(touchRead(TOUCH_1_Seg_PIN)));
oled.setCursor(0, 33);
    //display.println("Second Seg " + String(touchRead(TOUCH_2_Seg_PIN)));
oled.setCursor(0, 43);
   // display.println("Third Seg " + String(touchRead(TOUCH_3_Seg_PIN)));
oled.setCursor(0, 53);
    //display.println("Fourth Seg " + String(touchRead(TOUCH_4_Seg_PIN)));
oled.setCursor(0, 63);
   // display.println("Fourth Seg " + String(touchRead(TOUCH_5_Seg_PIN)));
oled.setFont(&DejaVu_LGC_Sans_Bold_10);
    Serial.println("Starting Display");
    oledMana.display();
    Serial.println("Finished Display");
    delay(10);
}
