#include "weather.h"

bool isWeatherAvailable = false;
savedDailyForecastData weatherDailyForecastData[MAX_DAYS]; // Days / Data for these days
CurrentWeatherData currentWeatherData;

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
    uint32_t lastDailyWeatherSync = 0;
    uint32_t lastCurrentWeatherSync = 0;

    if (isWeatherAvailable == false)
    {
        syncDailyWeather();
        vTaskDelay(pdMS_TO_TICKS(10000));
        syncCurrentWeather();
    }

    while (true) {
        uint32_t currentTime = millis();

        // Sync daily weather if the interval has passed
        if (currentTime - lastDailyWeatherSync >= DAILY_WEATHER_INTERVAL) {
            if (WiFi.status() == WL_CONNECTED) {
                syncDailyWeather();
                Serial.println("Daily weather synced.");
                lastDailyWeatherSync = currentTime;
            } else {
                Serial.println("WiFi not connected. Cannot sync daily weather.");
            }
        }
        vTaskDelay(pdMS_TO_TICKS(5000));
        // Sync current weather if the interval has passed
        if (currentTime - lastCurrentWeatherSync >= CURRENT_WEATHER_INTERVAL) {
            if (WiFi.status() == WL_CONNECTED) {
                syncCurrentWeather();
                Serial.println("Current weather synced.");
                lastCurrentWeatherSync = currentTime;
            } else {
                Serial.println("WiFi not connected. Cannot sync current weather.");
            }
        }
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

void syncDailyWeather()
{
    // Dynamically allocate OM_DailyForecast
    OM_DailyForecast* omdailyForecast = new OM_DailyForecast;

    // Get the daily forecast data
    getDailyForecast(omdailyForecast, WEATHER_LATIT, WEATHER_LONGTIT);

    // Update weatherDailyForecastData with the retrieved data
    for (int i = 0; i < MAX_DAYS; i++)
    {
        weatherDailyForecastData[i].dt = omdailyForecast->daily_time[i];
        weatherDailyForecastData[i].windDirection = omdailyForecast->wind_deg_dominant[i];
        weatherDailyForecastData[i].temp = (omdailyForecast->apparent_temp_max[i] + omdailyForecast->apparent_temp_min[i]) / 2;
        weatherDailyForecastData[i].minTemp = omdailyForecast->temp_min[i];
        weatherDailyForecastData[i].maxTemp = omdailyForecast->temp_max[i];
        weatherDailyForecastData[i].weatherConditionId = omdailyForecast->weather_code[i];
        weatherDailyForecastData[i].windSpeed = omdailyForecast->wind_speed_max[i];
        weatherDailyForecastData[i].windGusts = omdailyForecast->wind_gust_max[i];
        weatherDailyForecastData[i].pop = omdailyForecast->precipitation_max[i];
        weatherDailyForecastData[i].sunrise = omdailyForecast->sunrise[i];
        weatherDailyForecastData[i].sunset = omdailyForecast->sunset[i];
    }

    // Deallocate the memory to prevent memory leaks
    delete omdailyForecast;

    // Mark the weather data as available
    isWeatherAvailable = true;
}



void syncCurrentWeather() {
    // Dynamically allocate OM_CurrentWeather
    OM_CurrentWeather* omCurrentWeather = new OM_CurrentWeather;

    // Get the current weather data
    getCurrentWeather(omCurrentWeather, WEATHER_LATIT, WEATHER_LONGTIT);

    // Populate the CurrentWeatherData structure
    currentWeatherData.dt = omCurrentWeather->time;
    currentWeatherData.main = weatherConditionIdToStr(omCurrentWeather->weather_code);
    currentWeatherData.windDirection = omCurrentWeather->wind_deg;
    currentWeatherData.temp = omCurrentWeather->temp;
    currentWeatherData.pressure = omCurrentWeather->pressure;
    currentWeatherData.humidity = omCurrentWeather->humidity;
    currentWeatherData.cloudsPerc = omCurrentWeather->cloud_cover;
    currentWeatherData.windSpeed = omCurrentWeather->wind_speed;
    currentWeatherData.windGusts = omCurrentWeather->wind_gust;
    currentWeatherData.weatherConditionId = omCurrentWeather->weather_code;

    // Deallocate the memory to prevent memory leaks
    delete omCurrentWeather;

    // Mark the weather data as available
    isWeatherAvailable = true;
}



String weatherConditionIdToStr(int code_no)
{
    switch (code_no)
    {
        case 0:  return "Clear sky";
        case 1: case 2: case 3: return "Partly cloudy";
        case 45: case 48: return "Fog";
        case 51: case 53: case 55: return "Drizzle";
        case 56: case 57: return "Freezing drizzle";
        case 61: case 63: case 65: return "Rain";
        case 66: case 67: return "Freezing rain";
        case 71: case 73: case 75: return "Snow";
        case 77: return "Snow grains";
        case 80: case 81: case 82: return "Rain showers";
        case 85: case 86: return "Snow showers";
        case 95: return "Thunderstorm";
        case 96: case 99: return "Thunderstorm with hail";
        default: return "Unknown";
    }
}
