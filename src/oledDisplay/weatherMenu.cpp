#include "weatherMenu.h"

void displaywidget(int code_no);
String convertWindDirection(uint16_t degrees);

void todaysWeather()
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
    display.setCursor(0, SCREEN_HEIGHT - 1);
    display.setFont(&Roboto_Black_9);
    display.print(weatherConditionIdToStr(weatherForecastData[0]->weatherConditionId));
    display.print("\t");
    display.print("");
    display.startscrollleft(0x07, 0x07);
    display.display();
    display.setFont(&DejaVu_LGC_Sans_Bold_10);
}

void tommorowsWeather()
{
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setFont(&DejaVu_LGC_Sans_Bold_10);
    displaywidget(weatherForecastData[1][4].weatherConditionId);
    display.setTextSize(1);
    display.setCursor(54, 20);
    display.print(getWeekdayName(weekday(now()) + 1)); // Prints city name ie: Marmagao in my case
    display.drawLine(57, 25, 127, 25, WHITE);

    float tmax = -9999;
    float tmin = 9999;
    for (int i = 0; i < 8; i++)
    {
        if (weatherForecastData[1][i].maxTemp > tmax)
            tmax = weatherForecastData[1][i].maxTemp;
        if (weatherForecastData[1][i].minTemp < tmin)
            tmin = weatherForecastData[1][i].minTemp;
    }

    display.setCursor(57, 35);
    display.print("Min " + String(tmin) + "C");
    display.setCursor(57, 45);
    display.print("Max " + String(tmax) + "C");
    display.setCursor(57, 55);
    display.print(weatherForecastData[1][4].windSpeed);
    display.print("m/s ");
    display.println(convertWindDirection(weatherForecastData[1][4].windDirection));
    display.setCursor(0, SCREEN_HEIGHT - 1);
    display.setFont(&Roboto_Black_9);
    display.print(weatherConditionIdToStr(weatherForecastData[1][4].weatherConditionId));
    display.print("\t");
    display.print("");
    display.startscrollleft(0x07, 0x07);
    display.display();
    display.setFont(&DejaVu_LGC_Sans_Bold_10);
}

void daysAfterWeather() {
        display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setFont(&DejaVu_LGC_Sans_Bold_10);
    displaywidget(weatherForecastData[2][4].weatherConditionId);
    display.setTextSize(1);
    display.setCursor(54, 20);
    display.print(getWeekdayName(weekday(now()) + 2)); // Prints city name ie: Marmagao in my case
    display.drawLine(57, 25, 127, 25, WHITE);

    float tmax = -9999;
    float tmin = 9999;
    for (int i = 0; i < 8; i++)
    {
        if (weatherForecastData[2][i].maxTemp > tmax)
            tmax = weatherForecastData[2][i].maxTemp;
        if (weatherForecastData[2][i].minTemp < tmin)
            tmin = weatherForecastData[2][i].minTemp;
    }

    display.setCursor(57, 35);
    display.print("Min " + String(tmin) + "C");
    display.setCursor(57, 45);
    display.print("Max " + String(tmax) + "C");
    display.setCursor(57, 55);
    display.print(weatherForecastData[2][4].windSpeed);
    display.print("m/s ");
    display.println(convertWindDirection(weatherForecastData[2][4].windDirection));
    display.setCursor(0, SCREEN_HEIGHT - 1);
    display.setFont(&Roboto_Black_9);
    display.print(weatherConditionIdToStr(weatherForecastData[2][4].weatherConditionId));
    display.print("\t");
    display.print("");
    display.startscrollleft(0x07, 0x07);
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
        display.fillCircle(30, 30, 20, WHITE); // sunny day
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
        return "Unknown";
    }
}
