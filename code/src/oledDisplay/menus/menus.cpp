#include "menus.h"

void checkExit()
{
    if (shouldExitLoop() == true)
    {
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
    oled.clearDisplay(); // Clear the display

    // 1. Top Section: Weather Widget (Left: 0..48) & Header Info (Right: 52..127)
    displaywidget(currentWeatherData.weatherConditionId);

    oled.setTextSize(1);
    oled.setFont(&DejaVu_LGC_Sans_Bold_9);
    oled.setTextColor(SSD1327_WHITE);

    // Date & Day
    oled.setCursor(52, 10);
    oled.print(String(day()) + "." + String(month()) + "." + String(year()));

    oled.setCursor(52, 22);
    oled.print(getCurrentWeekdayName());

    oled.setTextColor(12); // Slightly dimmed
    oled.setCursor(52, 34);
    oled.print(currentWeatherData.isDay ? "Daytime" : "Nighttime");

    // Dim separator line (gray value 6 out of 15)
    oled.drawLine(0, 42, 127, 42, 6);

    // 2. Middle Section: Temperature & Clouds
    oled.setTextColor(SSD1327_WHITE);
    oled.setFont(&DejaVu_Sans_Bold_16);
    oled.setCursor(2, 65);
    oled.print(String(currentWeatherData.temp, 1));

    // Lower/Smaller "°C" for temperature unit
    oled.setFont(&DejaVu_LGC_Sans_Bold_9);
    oled.print(" °C");

    // Cloud coverage on right side
    oled.setCursor(72, 57);
    oled.print("Clouds:");
    oled.setCursor(72, 68);
    oled.setTextColor(12);
    oled.print(String(currentWeatherData.cloudsPerc) + "%");

    // Dim separator line
    oled.drawLine(0, 74, 127, 74, 6);

    // 3. Data Grid: Compact labels to prevent line wrapping
    oled.setTextColor(SSD1327_WHITE);
    oled.setFont(&DejaVu_LGC_Sans_Bold_9);

    // Row 1: Wind speed, direction & Gusts
    oled.setCursor(2, 86);
    oled.print("Wind: ");
    oled.print(currentWeatherData.windSpeed, 1);
    oled.print("m/s ");
    oled.print(convertWindDirection(currentWeatherData.windDirection));

    oled.setCursor(2, 97);
    oled.print("Gust: ");
    oled.print(currentWeatherData.windGusts, 1);
    oled.print("m/s");

    // Row 2: Humidity & Pressure (Shortened to H and P, shifted left to fit)
    oled.setCursor(2, 108);
    oled.print("H: ");
    oled.print(currentWeatherData.humidity);
    oled.print("%");

    oled.setCursor(52, 108); // Shifted left to x=52 to guarantee no overflow for "P: 1013hPa"
    oled.print("P: ");
    oled.print((int)currentWeatherData.pressure);
    oled.print("hPa");

    // 4. Footer Section: Bottom status description
    oled.drawLine(0, 114, 127, 114, 6); // Dim separator line

    oled.setFont(&Roboto_Black_9);
    oled.setCursor(3, 124);
    oled.setTextColor(SSD1327_WHITE);

    if (isWeatherAvailable == true)
    {
        oled.print(currentWeatherData.main);
    }
    else
    {
        oled.print("N/A");
    }

    // Refresh display
    delay(10);
    oledMana.display();

    // Restore default font
    oled.setFont(&DejaVu_LGC_Sans_Bold_10);
}

void displayWeatherCast(int dayIndex)
{
    checkExit();

    oled.clearDisplay();

    // 1. Top Section: Widget (Left) & Forecast Day Header (Right)
    displaywidget(weatherDailyForecastData[dayIndex].weatherConditionId);

    oled.setTextSize(1);
    oled.setFont(&DejaVu_LGC_Sans_Bold_9);
    oled.setTextColor(SSD1327_WHITE);
    oled.setCursor(52, 18);
    oled.print(getNextDayName(dayIndex));

    oled.setTextColor(12);
    oled.setCursor(52, 30);
    oled.print("P(Rain): ");
    oled.print(weatherDailyForecastData[dayIndex].pop);
    oled.print("%");

    // Dim separator line
    oled.drawLine(0, 42, 127, 42, 6);

    // 2. Temperatures: Main Average Temp with Compact Min/Max
    float tmax = weatherDailyForecastData[dayIndex].maxTemp;
    float tmin = weatherDailyForecastData[dayIndex].minTemp;
    float tavg = (tmax + tmin) / 2.0;

    oled.setTextColor(SSD1327_WHITE);
    oled.setFont(&DejaVu_Sans_Bold_16);
    oled.setCursor(2, 65);
    oled.print(String(tavg, 1));

    // Lower/Smaller "°C"
    oled.setFont(&DejaVu_LGC_Sans_Bold_9);
    oled.print(" °C");

    // Min & Max temperatures
    oled.setCursor(72, 57);
    oled.print("Mx: ");
    oled.print(String(tmax, 1));
    oled.print("°");

    oled.setCursor(72, 68);
    oled.setTextColor(12);
    oled.print("Mn: ");
    oled.print(String(tmin, 1));
    oled.print("°");

    // Dim separator line
    oled.drawLine(0, 74, 127, 74, 6);

    // 3. Extended Daily Data: Wind, Gusts, Sunrise
    oled.setTextColor(SSD1327_WHITE);
    oled.setFont(&DejaVu_LGC_Sans_Bold_9);

    // Row 1: Wind & Gusts
    oled.setCursor(2, 86);
    oled.print("Wind: ");
    oled.print(weatherDailyForecastData[dayIndex].windSpeed, 1);
    oled.print("m/s ");
    oled.print(convertWindDirection(weatherDailyForecastData[dayIndex].windDirection));

    oled.setCursor(2, 97);
    oled.print("Gust: ");
    oled.print(weatherDailyForecastData[dayIndex].windGusts, 1);
    oled.print("m/s");

    // Row 2: Sunrise formatted as HH:MM directly from UNIX timestamp
    uint32_t sr = weatherDailyForecastData[dayIndex].sunrise;
    int srHour = hour(sr);
    int srMin = minute(sr);
    String sunriseStr = (srHour < 10 ? "0" : "") + String(srHour) + ":" + (srMin < 10 ? "0" : "") + String(srMin);

    uint32_t se = weatherDailyForecastData[dayIndex].sunset;
    int seHour = hour(se);
    int seMin = minute(se);
    String sunsetStr = (seHour < 10 ? "0" : "") + String(seHour) + ":" + (seMin < 10 ? "0" : "") + String(seMin);

    oled.setCursor(67, 108);
    oled.setTextColor(12);
    oled.print("Set: ");
    oled.print(sunsetStr);

    oled.setCursor(2, 108); // Shifted left to x=52 to match grid alignment
    oled.print("Sun: ");
    oled.print(sunriseStr);

    // 4. Footer Section: Bottom status description
    oled.drawLine(0, 114, 127, 114, 6); // Dim separator line

    oled.setFont(&Roboto_Black_9);
    oled.setCursor(3, 124);
    oled.setTextColor(SSD1327_WHITE);

    if (isWeatherAvailable == true)
    {
        oled.print(weatherConditionIdToStr(weatherDailyForecastData[dayIndex].weatherConditionId));
    }
    else
    {
        oled.print("N/A");
    }

    oledMana.display();
    oled.setFont(&DejaVu_LGC_Sans_Bold_10);
}

String padZero(int number)
{
    return (number < 10) ? ("0" + String(number)) : String(number);
}

void updateWeatherMenuDates()
{
    if (data.isSubmenu && data.currentSubmenu != nullptr)
    {
        time_t currentTime = now();

        // Indices 2 through 7 map to forecast days (+1 to +6 days offset)
        for (int i = 2; i < data.submenuCount; i++)
        {
            int dayOffset = i - 1;
            time_t targetTime = currentTime + (dayOffset * 86400UL);

            String dayName = getShortWeekdayName(weekday(targetTime));

            String dateString = dayName + " " +
                                padZero(day(targetTime)) + "." +
                                padZero(month(targetTime)) + "." +
                                String(year(targetTime));

            data.currentSubmenu[i].text = dateString;
        }
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

    // 1. Header Section
    oled.setFont(&DejaVu_LGC_Sans_Bold_9);
    oled.setTextColor(SSD1327_WHITE);
    centerText("WiFi Debug", 11);
    oled.drawLine(0, 15, 127, 15, 6); // Dim separator

    // 2. Data List
    oled.setFont(&DejaVu_LGC_Sans_Bold_9);

    // SSID
    oled.setCursor(2, 28);
    oled.setTextColor(SSD1327_WHITE);
    oled.print("SSID:");
    oled.setCursor(2, 39);
    oled.setTextColor(12);
    oled.print(WiFi.SSID().length() > 0 ? WiFi.SSID() : "Disconnected");

    // IP Address
    oled.setCursor(2, 53);
    oled.setTextColor(SSD1327_WHITE);
    oled.print("IP Address:");
    oled.setCursor(2, 64);
    oled.setTextColor(12);
    oled.print(WiFi.localIP().toString());

    // MAC Address
    oled.setCursor(2, 78);
    oled.setTextColor(SSD1327_WHITE);
    oled.print("MAC Address:");
    oled.setCursor(2, 89);
    oled.setTextColor(12);
    oled.print(WiFi.macAddress());

    // Signal & Channel
    oled.setCursor(2, 103);
    oled.setTextColor(SSD1327_WHITE);
    oled.print("RSSI: ");
    oled.setTextColor(12);
    oled.print(WiFi.RSSI());
    oled.setTextColor(6);
    oled.print(" dBm");

    // 3. Footer Line
    oled.drawLine(0, 114, 127, 114, 6);

    oledMana.display();
    delay(10);
    oled.setFont(&DejaVu_LGC_Sans_Bold_10);
}
void CPUDebugMenu()
{
    checkExit();
    oled.clearDisplay();

    // 1. Header Section
    oled.setFont(&DejaVu_LGC_Sans_Bold_9);
    oled.setTextColor(SSD1327_WHITE);
    centerText("CPU Debug", 11);
    oled.drawLine(0, 15, 127, 15, 6);

    // 2. Data List
    oled.setFont(&DejaVu_LGC_Sans_Bold_9);

    // Chip Model
    oled.setCursor(2, 28);
    oled.setTextColor(SSD1327_WHITE);
    oled.print("Chip Model:");
    oled.setCursor(2, 39);
    oled.setTextColor(12);
    oled.print(ESP.getChipModel());

    // CPU Frequency
    oled.setCursor(2, 53);
    oled.setTextColor(SSD1327_WHITE);
    oled.print("CPU Freq: ");
    oled.setTextColor(12);
    oled.print(getCpuFrequencyMhz());
    oled.setTextColor(6);
    oled.print(" MHz");

    // XTAL Frequency
    oled.setCursor(2, 67);
    oled.setTextColor(SSD1327_WHITE);
    oled.print("XTAL Freq: ");
    oled.setTextColor(12);
    oled.print(getXtalFrequencyMhz());
    oled.setTextColor(6);
    oled.print(" MHz");

    // Cores & Revision
    oled.setCursor(2, 81);
    oled.setTextColor(SSD1327_WHITE);
    oled.print("Cores: ");
    oled.setTextColor(12);
    oled.print(ESP.getChipCores());

    oled.setCursor(2, 95);
    oled.setTextColor(SSD1327_WHITE);
    oled.print("Rev: ");
    oled.setTextColor(12);
    oled.print(ESP.getChipRevision());

    // 3. Footer Line
    oled.drawLine(0, 114, 127, 114, 6);

    oledMana.display();
    delay(10);
    oled.setFont(&DejaVu_LGC_Sans_Bold_10);
}
void generalDebugMenu()
{
    checkExit();
    oled.clearDisplay();

    // 1. Header Section
    oled.setFont(&DejaVu_LGC_Sans_Bold_9);
    oled.setTextColor(SSD1327_WHITE);
    centerText("General Debug", 11);
    oled.drawLine(0, 15, 127, 15, 6);

    // 2. Data List
    oled.setFont(&DejaVu_LGC_Sans_Bold_9);

    // Reset Reason
    oled.setCursor(2, 28);
    oled.setTextColor(SSD1327_WHITE);
    oled.print("Reset Reason:");
    oled.setCursor(2, 39);
    oled.setTextColor(12);
    oled.print(resetReasonToString(esp_reset_reason()));

    // Uptime
    oled.setCursor(2, 53);
    oled.setTextColor(SSD1327_WHITE);
    oled.print("Uptime: ");
    oled.setTextColor(12);
    oled.print(microsecondsToTimeString(esp_timer_get_time()));

    // Free Heap
    oled.setCursor(2, 67);
    oled.setTextColor(SSD1327_WHITE);
    oled.print("Free Heap: ");
    oled.setTextColor(12);
    oled.print(esp_get_free_heap_size() / 1024.0, 1);
    oled.setTextColor(6);
    oled.print(" KB");

    // Battery Voltage
    oled.setCursor(2, 81);
    oled.setTextColor(SSD1327_WHITE);
    oled.print("Battery: ");
    oled.setTextColor(12);
    oled.print(getBatteryVoltage(), 2);
    oled.setTextColor(6);
    oled.print(" V");

    // Charge Indicator Indicator Status
    if (charging)
    {
        oled.setCursor(95, 81);
        oled.setTextColor(SSD1327_WHITE);
        oled.print("[CHG]");
    }

    // 3. Footer Line
    oled.drawLine(0, 114, 127, 114, 6);

    oledMana.display();
    delay(10);
    oled.setFont(&DejaVu_LGC_Sans_Bold_10);
}