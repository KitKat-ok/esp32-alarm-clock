#include "menus.h"

void displaywidget(int code_no);
String convertWindDirection(uint16_t degrees);

void currentWeather()
{
    display.clearDisplay();
    display.setFont(&DejaVu_LGC_Sans_Bold_10);
    displaywidget(weatherForecastData[0]->weatherConditionId);
    display.setTextSize(1);
    display.setCursor(54, 20);
    display.print(getCurrentWeekdayName()); // Prints city name ie: Marmagao in my case
    display.drawLine(58, 25, 127, 25, WHITE);

    display.setCursor(58, 40);
    display.setFont(&DejaVu_Sans_Bold_16);
    display.print(String(weatherForecastData[0]->temp) + "C");
    display.setFont(&DejaVu_LGC_Sans_Bold_10);
    display.setCursor(58, 48);
    display.print(weatherForecastData[0]->windSpeed);
    display.print("m/s ");
    display.println(convertWindDirection(weatherForecastData[0]->windDirection));
    display.setCursor(0, SCREEN_HEIGHT - 5);
    display.setFont(&Roboto_Black_9);
    display.print(weatherConditionIdToStr(weatherForecastData[0]->weatherConditionId));
    display.print("\t");
    display.print("");
    display.startscrollleft(0x06, 0x07);
    display.display();
    display.setFont(&DejaVu_LGC_Sans_Bold_10);
}

void displayWeatherCast(int dayIndex)
{
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    displaywidget(weatherForecastData[dayIndex][4].weatherConditionId);
    display.setTextSize(1);
    display.setCursor(55, 10);
    display.setFont(&DejaVu_LGC_Sans_Bold_9);
    display.print(getNextDayName(dayIndex));
    display.drawLine(57, 15, 127, 15, WHITE);
    display.setFont(&DejaVu_LGC_Sans_Bold_10);

    float tmax = -9999;
    float tmin = 9999;
    for (int i = 0; i < 8; i++)
    {
        if (weatherForecastData[dayIndex][i].maxTemp > tmax)
            tmax = weatherForecastData[dayIndex][i].maxTemp;
        if (weatherForecastData[dayIndex][i].minTemp < tmin)
            tmin = weatherForecastData[dayIndex][i].minTemp;
    }

    display.setCursor(57, 25);
    display.print("Min " + String(tmin) + "");
    display.setCursor(57, 35);
    display.print("Max " + String(tmax) + "");

    display.setCursor(57, 45);
    display.print(weatherForecastData[dayIndex][4].windSpeed);
    display.print("m/s ");
    display.println(convertWindDirection(weatherForecastData[dayIndex][4].windDirection));

    display.setCursor(0, SCREEN_HEIGHT - 5);
    display.setFont(&DejaVu_LGC_Sans_Bold_9);
    display.print(weatherConditionIdToStr(weatherForecastData[dayIndex][4].weatherConditionId));
    display.print("");

    display.startscrollleft(0x06, 0x07);
    display.display();
    display.setFont(&DejaVu_LGC_Sans_Bold_10);
}

void displaywidget(int code_no)
{
    if (200 <= code_no and code_no <= 202)
        display.drawBitmap(0, 0, thunderstorm_rain, 57, 56, BLACK, WHITE);
    else if (230 <= code_no and code_no <= 233)
        display.drawBitmap(0, 0, thunderstrom_drizzle, 57, 56, BLACK, WHITE);
    else if (300 <= code_no and code_no <= 302)
        display.drawBitmap(0, 0, drizzle, 57, 54, BLACK, WHITE);
    else if (500 <= code_no and code_no <= 522)
        display.drawBitmap(0, 0, rain, 57, 54, BLACK, WHITE);
    else if ((600 <= code_no and code_no <= 610) or (621 <= code_no and code_no <= 623))
        display.drawBitmap(0, 0, snow, 57, 54, BLACK, WHITE);
    else if (611 <= code_no and code_no <= 612)
        display.drawBitmap(0, 0, sleet, 57, 54, BLACK, WHITE);
    else if (700 <= code_no and code_no <= 751)
        display.drawBitmap(0, 0, mist, 57, 54, BLACK, WHITE);
    else if (801 <= code_no and code_no <= 804)
        display.drawBitmap(0, 0, cloudy, 57, 54, BLACK, WHITE);
    else if (code_no == 900)
        display.drawBitmap(0, 0, unknown_prep, 57, 54, BLACK, WHITE);
    else
        display.fillCircle(30, 23, 20, WHITE); // sunny day
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
    display.display();
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
    display.display();
}

void generalDebugMenu()
{
    display.clearDisplay();
    centerText("General debug", 10);
    display.drawRect(0, SCREEN_HEIGHT / 3 - 8, SCREEN_WIDTH, 2, SSD1306_WHITE);
    display.setFont(&DejaVu_LGC_Sans_Bold_9);
    centerText("Reset reason:", 24);
    centerText(resetReasonToString(esp_reset_reason()), 34);
    display.setCursor(0, 45);
    display.println("Uptime: " + microsecondsToTimeString(esp_timer_get_time()));
    display.setCursor(0, 55);
    display.println("Free heap: " + String(esp_get_free_heap_size() / 1024.0, 2) + " KB");
    display.setCursor(0, 65);
    display.setFont(&DejaVu_LGC_Sans_Bold_10);
    display.display();
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

    display.display();
}
