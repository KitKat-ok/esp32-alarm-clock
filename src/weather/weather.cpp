#include "weather.h"

OW_Weather ow;
bool isWeatherAvailable = false;
OW_forecast *forecast = NULL;
savedForecastData weatherForecastData[MAX_DAYS][WEATHER_PER_DAY]; // Days / Data for these days

TaskHandle_t WeatherTask;

void weatherTask(void *parameter);
void syncWeather();
void dumpWeather();

void createWeatherTask()
{
    xTaskCreatePinnedToCore(
        weatherTask,       // Function to implement the task
        "Syncing Weather", // Name of the task
        10000,             // Stack size (words)
        NULL,              // Parameter to pass
        1,                 // Priority
        &WeatherTask,      // Task handle
        1                  // Core to run the task on (Core 1)
    );
}

void deleteWeatherTask()
{
    vTaskDelete(WeatherTask);
}

void weatherTask(void *parameter)
{
    while (true)
    {
        Serial.println("Syncing weather");
        syncWeather();
        vTaskDelay(pdMS_TO_TICKS(30 * 60 * 1000));
    }
}

void syncWeather()
{
    if (OPEN_WEATHER_API_KEY != "" && WEATHER_LONGTIT != "" && WEATHER_LATIT != "")
    {
        forecast = new OW_forecast;

        bool status = ow.getForecast(forecast, OPEN_WEATHER_API_KEY, WEATHER_LATIT, WEATHER_LONGTIT, WEATHER_UNIT, WEATHER_LANG, false);
        if (status == true && forecast)
        {
            uint8_t c = 0;
            for (int i = 0; i < MAX_DAYS; i++)
            {
                for (int j = 0; j < WEATHER_PER_DAY; j++)
                {
                    weatherForecastData[i][j].dt = forecast->dt[c];
                    weatherForecastData[i][j].main = forecast->main[c];
                    weatherForecastData[i][j].icon = forecast->icon[c];
                    weatherForecastData[i][j].windDirection = forecast->wind_deg[c];
                    weatherForecastData[i][j].temp = forecast->feels_like[c];
                    weatherForecastData[i][j].minTemp = forecast->temp_min[c];
                    weatherForecastData[i][j].maxTemp = forecast->temp_max[c];
                    weatherForecastData[i][j].pressure = forecast->pressure[c];
                    weatherForecastData[i][j].humidity = forecast->humidity[c];
                    weatherForecastData[i][j].weatherConditionId = forecast->id[c];
                    weatherForecastData[i][j].cloudsPerc = forecast->clouds_all[c];
                    weatherForecastData[i][j].windSpeed = forecast->wind_speed[c];
                    weatherForecastData[i][j].windGusts = forecast->wind_gust[c];
                    weatherForecastData[i][j].visibility = forecast->visibility[c];
                    weatherForecastData[i][j].pop = forecast->pop[c] * 100;
                    weatherForecastData[i][j].sunrise = forecast->sunrise;
                    weatherForecastData[i][j].sunset = forecast->sunset;
                    c = c + 1;
                }
            }
            isWeatherAvailable = true;
            delete forecast;
        }
        else
        {
            isWeatherAvailable = false;
        }
    }
    else
    {
        isWeatherAvailable = false;
    }
}

String weatherConditionIdToStr(int weatherCode)
{
    switch (weatherCode)
    {
    // Group 2xx: Thunderstorm
    case 200:
        return "Thunderstorm with light rain";
    case 201:
        return "Thunderstorm with rain";
    case 202:
        return "Thunderstorm with heavy rain";
    case 210:
        return "Light thunderstorm";
    case 211:
        return "Thunderstorm";
    case 212:
        return "Heavy thunderstorm";
    case 221:
        return "Ragged thunderstorm";
    case 230:
        return "Thunderstorm with light drizzle";
    case 231:
        return "Thunderstorm with drizzle";
    case 232:
        return "Thunderstorm with heavy drizzle";

    // Group 3xx: Drizzle
    case 300:
        return "Light intensity drizzle";
    case 301:
        return "Drizzle";
    case 302:
        return "Heavy intensity drizzle";
    case 310:
        return "Light intensity drizzle rain";
    case 311:
        return "Drizzle rain";
    case 312:
        return "Heavy intensity drizzle rain";
    case 313:
        return "Shower rain and drizzle";
    case 314:
        return "Heavy shower rain and drizzle";
    case 321:
        return "Shower drizzle";

    // Group 5xx: Rain
    case 500:
        return "Light rain";
    case 501:
        return "Moderate rain";
    case 502:
        return "Heavy intensity rain";
    case 503:
        return "Very heavy rain";
    case 504:
        return "Extreme rain";
    case 511:
        return "Freezing rain";
    case 520:
        return "Light intensity shower rain";
    case 521:
        return "Shower rain";
    case 522:
        return "Heavy intensity shower rain";
    case 531:
        return "Ragged shower rain";

    // Group 6xx: Snow
    case 600:
        return "Light snow";
    case 601:
        return "Snow";
    case 602:
        return "Heavy snow";
    case 611:
        return "Sleet";
    case 612:
        return "Light shower sleet";
    case 613:
        return "Shower sleet";
    case 615:
        return "Light rain and snow";
    case 616:
        return "Rain and snow";
    case 620:
        return "Light shower snow";
    case 621:
        return "Shower snow";
    case 622:
        return "Heavy shower snow";

    // Group 7xx: Atmosphere
    case 701:
        return "Mist";
    case 711:
        return "Smoke";
    case 721:
        return "Haze";
    case 731:
        return "Sand/dust whirls";
    case 741:
        return "Fog";
    case 751:
        return "Sand";
    case 761:
        return "Dust";
    case 762:
        return "Volcanic ash";
    case 771:
        return "Squalls";
    case 781:
        return "Tornado";

    // Group 800: Clear
    case 800:
        return "Clear sky";

    // Group 80x: Clouds
    case 801:
        return "Few clouds: 11-25%";
    case 802:
        return "Scattered clouds: 25-50%";
    case 803:
        return "Broken clouds: 51-84%";
    case 804:
        return "Overcast clouds: 85-100%";

    // Default case
    default:
        return "Unknown weather code";
    }
}
