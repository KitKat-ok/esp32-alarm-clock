#ifndef WEATHER_H
#define WEATHER_H

#include "defines.h"


struct savedForecastData {
    uint32_t dt; // time, unix, use strTime
    String main;
    String icon;
    uint16_t windDirection;
    float temp; // C, depends
    float minTemp; // C, depends
    float maxTemp;
    float pressure; // hPa
    uint8_t humidity; // %
    uint16_t weatherConditionId; // Use weatherConditionIdToStr
    uint8_t cloudsPerc; // % of clouds
    float windSpeed; // m/s
    float windGusts; // m/s
    uint32_t visibility;
    uint8_t pop; // Probability of precipitation in %
    uint32_t sunrise; // time, unix
    uint32_t sunset; // time, unix
};

extern bool isWeatherAvailable;
extern savedForecastData weatherForecastData[MAX_DAYS][WEATHER_PER_DAY]; // Days / Data for these days

void syncWeather();

void deleteWeatherTask();

void createWeatherTask();

String weatherConditionIdToStr(int weatherCode);


#endif
