#ifndef DEFINES_H
#define DEFINES_H

#include "fonts/fonts.h"
#include "weather/weathericons.h"

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <TM1637Display.h>
#include <BH1750.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <TimeLib.h>
#include <NTPClient.h>
#include <Preferences.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <oledManager.h>

extern bool OTAEnabled;

// Led Display
#define CLK  19 
#define DIO  18 

// NTP
#define TIME_OFFSET_S 3600            // Time offset in seconds, use this as timezones

// Weather
#define DAILY_WEATHER_INTERVAL 5 * 60 * 60 * 1000
#define CURRENT_WEATHER_INTERVAL 30 * 60 * 1000  // 30 minutes

// Light Sensor
#define OLED_DISABLE_THRESHOLD 45
#define LED_DIM_THRESHOLD 30
#define LED_DISABLE_THRESHOLD 1

#define MAX_INCREASE_OF_LIGHT_LEVEL 1000

// Inputs
#define BUTTON_UP_PIN      GPIO_NUM_12
#define BUTTON_DOWN_PIN    GPIO_NUM_13
#define BUTTON_CONFIRM_PIN GPIO_NUM_14
#define BUTTON_EXIT_PIN    GPIO_NUM_15

#define MAX_MENU_ITEMS 15
#define MAX_ALARMS 15


#define TOUCH_BUTTON_PIN 4
#define TOUCH_BUTTON_THRESHOLD 8
#define TOUCH_BUTTON_THRESHOLD_WHEN_ALREADY_TOUCHED 8
#define TOUCH_BUTTON_THRESHOLD_ON_BATTERY 9

#define NUM_TOUCH_SAMPLES 10

// Menus
#define TOTAL_NAV_BUTTONS 4
#define MAX_DEPTH 20

#define MAIN_PAGE_DURATION 60000
#define SCREENSAVER_DURATION 30000

#define N_FLYERS 5  // Numver of flyers on screensaver

#define fontX 6
//5
#define fontY 12

// Alarm
#define SUNDAY    0
#define MONDAY    1
#define TUESDAY   2
#define WEDNESDAY 3
#define THURSDAY  4
#define FRIDAY    5
#define SATURDAY  6

// Buzzer
#define BUZZER_PIN 27
//#define START_SOUND false // uncomment to enable start sound

// Battery
#define MIN_VOLTAGE 3.40  // Minimum voltage of LiPo battery
#define MAX_VOLTAGE 4.20  // Maximum voltage of LiPo battery
#define NUM_SAMPLES 15   // How many samples to read to smooth the voltage out

#define VOLTAGE_DIVIDER_PIN 34 

#define FULLY_CHARGED_PIN 32
#define CHARGING_PIN 33

#define TIMER_WAKUP_TIME 15000 // In miliseconds
#define GPIO_WAKUP_TIME 30000 // In miliseconds
#define SLEEPING_TIME 60 * 1000000 

#define CHARGING_THRESHOLD 1000
#define STANDBY_THRESHOLD 1000

// Temperature
#define TEMP_SENS_PIN 5
#define TEMP_CHART_READINGS 55 // Number of readings to keep

#define TEMP_OFFSET 2.77


#include "hardware/pitches.h"

#include "confidential.h"

#include "hardware/hardware.h"
#include "functions.h"
#include "WiFi/WiFi.h"
#include "NTP/NTP.h"
#include "weather/weather.h"
#include "LedDisplay/LedDisplay.h"
#include "lightSensor/lightSensor.h"
#include "oledDisplay/interface.h"
#include "oledDisplay/mainPage.h"
#include "oledDisplay/charts.h"
#include "oledDisplay/menus.h"
#include "Flash/Flash.h"
#include "alarms/alarms.h"
#include "battery/battery.h"
#include "tempSensor/tempSensor.h"


#endif
