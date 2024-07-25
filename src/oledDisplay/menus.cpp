#include "menus.h"



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
    display.fillRect(0,SCREEN_HEIGHT - 16,SCREEN_WIDTH,16,SSD1306_BLACK);
    display.print(currentWeatherData.main); // Print the weather condition description
    display.print("\t");
    display.print("");

    // Scroll the display content
    display.startscrollleft(0x06, 0x07);
    oledDisplay();

    // Restore the font settings
    display.setFont(&DejaVu_LGC_Sans_Bold_10);
}



void displayWeatherCast(int dayIndex)
{
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
    display.print("Min " + String(tmin) + "C");
    display.setCursor(57, 35);
    display.print("Max " + String(tmax) + "C");

    display.setCursor(57, 45);
    display.print(weatherDailyForecastData[dayIndex].windSpeed);
    display.print("m/s ");
    display.println(convertWindDirection(weatherDailyForecastData[dayIndex].windDirection));

    display.setCursor(0, SCREEN_HEIGHT - 5);
    display.setFont(&DejaVu_LGC_Sans_Bold_9);
    display.fillRect(0,SCREEN_HEIGHT - 16,SCREEN_WIDTH,16,SSD1306_BLACK);

    display.print(weatherConditionIdToStr(weatherDailyForecastData[dayIndex].weatherConditionId));
    display.print("");

    display.startscrollleft(0x06, 0x07);
    oledDisplay();
    display.setFont(&DejaVu_LGC_Sans_Bold_10);
}


void displaywidget(int code_no)
{
    switch (code_no)
    {
        case 0:
            display.fillCircle(30, 23, 20, WHITE); // Clear sky
            break;
        case 1:
        case 2:
        case 3:
            display.drawBitmap(0, 0, cloudy, 57, 54, BLACK, WHITE); // Mainly clear, partly cloudy, and overcast
            break;
        case 45:
        case 48:
            display.drawBitmap(0, 0, mist, 57, 54, BLACK, WHITE); // Fog and depositing rime fog
            break;
        case 51:
        case 53:
        case 55:
            display.drawBitmap(0, 0, drizzle, 57, 54, BLACK, WHITE); // Drizzle: Light, moderate, and dense intensity
            break;
        case 56:
        case 57:
            display.drawBitmap(0, 0, sleet, 57, 54, BLACK, WHITE); // Freezing Drizzle: Light and dense intensity
            break;
        case 61:
        case 63:
        case 65:
            display.drawBitmap(0, 0, rain, 57, 54, BLACK, WHITE); // Rain: Slight, moderate, and heavy intensity
            break;
        case 66:
        case 67:
            display.drawBitmap(0, 0, sleet, 57, 54, BLACK, WHITE); // Freezing Rain: Light and heavy intensity
            break;
        case 71:
        case 73:
        case 75:
            display.drawBitmap(0, 0, snow, 57, 54, BLACK, WHITE); // Snow fall: Slight, moderate, and heavy intensity
            break;
        case 77:
            display.drawBitmap(0, 0, snow, 57, 54, BLACK, WHITE); // Snow grains
            break;
        case 80:
        case 81:
        case 82:
            display.drawBitmap(0, 0, rain, 57, 54, BLACK, WHITE); // Rain showers: Slight, moderate, and violent
            break;
        case 85:
        case 86:
            display.drawBitmap(0, 0, snow, 57, 54, BLACK, WHITE); // Snow showers slight and heavy
            break;
        case 95:
            display.drawBitmap(0, 0, thunderstorm_rain, 57, 56, BLACK, WHITE); // Thunderstorm: Slight or moderate
            break;
        case 96:
        case 99:
            display.drawBitmap(0, 0, thunderstorm_rain, 57, 56, BLACK, WHITE); // Thunderstorm with slight and heavy hail
            break;
        default:
            display.drawBitmap(0, 0, unknown_prep, 57, 54, BLACK, WHITE); // Unknown or unhandled weather code
            break;
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

void wifiDebugMenu()
{
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
    oledDisplay();
}

esp_chip_info_t chip_info;

#include "hal/efuse_hal.h"

void CPUDebugMenu()
{
    display.clearDisplay();
    centerText("CPU debug", 10);
    display.drawRect(0, SCREEN_HEIGHT / 3 - 8, SCREEN_WIDTH, 2, SSD1306_WHITE);
    display.setFont(&DejaVu_LGC_Sans_Bold_9);
    display.setCursor(0, 24);
    display.println("CPU freq: " + String(getCpuFrequencyMhz()) + " Mhz");
    display.setCursor(0, 34);
    display.println("XTAL freq: " + String(getXtalFrequencyMhz()) + " Mhz");
    centerText("Chip model:", 44);
    centerText(String(ESP.getChipModel()), 54);
    display.setFont(&DejaVu_LGC_Sans_Bold_10);
    oledDisplay();
}

void generalDebugMenu()
{
    display.clearDisplay();
    centerText("General debug " , 10);
    display.drawRect(0, SCREEN_HEIGHT / 3 - 8, SCREEN_WIDTH, 2, SSD1306_WHITE);
    display.setFont(&DejaVu_LGC_Sans_Bold_9);
    centerText("Reset reason:", 23);
    centerText(resetReasonToString(esp_reset_reason()), 33);
    display.setCursor(0, 43);
    display.println("Uptime: " + microsecondsToTimeString(esp_timer_get_time()) + " " + touchRead(TOUCH_BUTTON_PIN));
    display.setCursor(0, 53);
    display.println("Free heap: " + String(esp_get_free_heap_size() / 1024.0, 2) + " KB");
    display.setCursor(0, 63);
    display.println("Battery Vol: " + String(batteryVoltage));
    if (charging == true)
    {
    display.fillCircle((SCREEN_WIDTH - 20),60, 3, SSD1306_WHITE);
    }
    display.setFont(&DejaVu_LGC_Sans_Bold_10);
    oledDisplay();
}

void loopCallendar()
{
}

int ZellerCongruence(int day, int month, int year)
{
    if (month < 3)
    {
        month += 12;
        year--;
    }
    int k = year % 100;
    int j = year / 100;
    int h = (day + (13 * (month + 1)) / 5 + k + k / 4 + j / 4 + 5 * j) % 7;
    return (h + 5) % 7; // 0 - Saturday, 1 - Sunday, ..., 6 - Friday
}

void showCalendar(int monthNumber, int yearNumber)
{
    display.clearDisplay();

    String days[] = {"Su", "Mo", "Tu", "We", "Th", "Fr", "Sa"};

    // Display the day names
    int yOffset = 10;
    int totalWidth = 0;
    for (int i = 0; i < 7; i++)
    {
        int16_t x1, y1;
        uint16_t w, h;
        display.getTextBounds(days[i], 0, 0, &x1, &y1, &w, &h);
        totalWidth += w;
    }
    int gap = (SCREEN_WIDTH - totalWidth) / 6;
    int startX = gap / 2;
    for (int i = 0; i < 7; i++)
    {
        int16_t x1, y1;
        uint16_t w, h;
        display.getTextBounds(days[i], 0, 0, &x1, &y1, &w, &h);
        display.setCursor(startX, yOffset);
        display.print(days[i]);
        startX += w + gap;
    }

    // Determine the number of days in the month
    int daysInMonth;
    switch (monthNumber)
    {
    case 4:
    case 6:
    case 9:
    case 11:
        daysInMonth = 30;
        break;
    case 2:
        if ((yearNumber % 4 == 0 && yearNumber % 100 != 0) || (yearNumber % 400 == 0))
            daysInMonth = 29; // Leap year
        else
            daysInMonth = 28; // Non-leap year
        break;
    default:
        daysInMonth = 31;
    }

    // Calculate the starting day of the week for the first day of the month
    int startDay = ZellerCongruence(1, monthNumber, yearNumber);

    // Display the dates
    int currentDay = 1;
    int row = 2;
    int col = startDay;
    while (currentDay <= daysInMonth)
    {
        // Get the width and height of the date text
        int16_t x1, y1;
        uint16_t w, h;
        display.getTextBounds("00", 0, 0, &x1, &y1, &w, &h);

        // Calculate position for the date
        int boxWidth = w + gap;
        int x = col * boxWidth + gap / 2;
        int y = row * (h + 2) + yOffset;

        // Format the day with leading zeros if necessary
        String formattedDay = (currentDay < 10) ? "0" + String(currentDay) : String(currentDay);

        // Display the date
        display.setCursor(x, y);
        display.print(formattedDay);

        // Move to the next column
        col++;
        if (col == 7)
        {
            col = 0;
            row++;
        }
        currentDay++;
    }

    oledDisplay();
}
