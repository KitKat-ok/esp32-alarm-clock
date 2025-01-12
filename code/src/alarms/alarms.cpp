#include "alarms.h"
#include <HTTPClient.h>

Alarm alarms[MAX_ALARMS];

void ringAlarm(void *parameter);
void createRiningingTask();

void sendOnPostRequest();
void sendOffPostRequest();

void checkAlarm(int alarmHours, int alarmMinutes);
void checkAlarmsTask(void *pvParameters);

void initialzeAlarmArray()
{
  for (int i = 0; i < MAX_ALARMS; i++)
  {
    alarms[i] = {false, false, 0, 0, 0, true};
  }
}

void createAlarmTask()
{
  xTaskCreatePinnedToCore(
      checkAlarmsTask, // Function to implement the task
      "Alarms",        // Task name
      10000,           // Stack size (words)
      NULL,            // Task input parameter
      1,               // Priority (0 is lowest)
      NULL,            // Task handle
      0                // Core to run the task on (0 or 1)
  );
}

void checkAlarms()
{
  int currentDay = weekday() - 1; // Adjust to 0-based index
  Serial.println("current day:" + String(currentDay));

  for (int i = 0; i < MAX_ALARMS; ++i)
  {
    if (alarms[i].day == currentDay && alarms[i].enabled == true && alarms[i].exists == true)
    {
      checkAlarm(alarms[i].hours, alarms[i].minutes);
    }
  }
  if (powerConnected == true)
  {
    vTaskDelay(pdMS_TO_TICKS(1 * 1000));
  }
  else
  {
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

void checkAlarmsTask(void *pvParameters)
{
  while (true)
  {
    checkAlarms();
  }
}

void disableAllAlarms()
{
  for (int i = 0; i < MAX_ALARMS; i++)
  {
    alarms[i].enabled = false;
  }
}

void enableAllAlarms()
{
  for (int i = 0; i < MAX_ALARMS; i++)
  {
    if (alarms[i].hours != 0 && alarms[i].minutes != 0)
    {
      alarms[i].enabled = true;
    }
  }
}

int lastRingingMinute = -1; // Initialized to an invalid minute
int lastRingingHour = -1;   // Initialized to an invalid hour
bool ringing = false;

void checkAlarm(int alarmHours, int alarmMinutes)
{
  // Get the current time
  int currentHours = hour();
  int currentMinutes = minute();
  Serial.print("Check Alarm Function");

  // Check if it's time for the alarm
  if (currentHours == alarmHours && currentMinutes == alarmMinutes && (ringing == false || lastRingingMinute != currentMinutes || lastRingingHour != currentHours))
  {
    Serial.print("Alarm! It's time to wake up on ");
    createRiningingTask();

    lastRingingMinute = currentMinutes;
    lastRingingHour = currentHours;
    ringing = true;
  }
}

TaskHandle_t Alarm;

void createRiningingTask()
{
  xTaskCreatePinnedToCore(
      ringAlarm,   // Function to implement the task
      "ringAlarm", // Name of the task
      10000,       // Stack size (words)
      NULL,        // Parameter to pass
      2,           // Priority
      &Alarm,      // Task handle
      0            // Core to run the task on (Core 0)
  );
}

void touchStopAlarm(int hour, bool ringOn)
{
  if (readHallSwitch() == true)
  {
    if (!(hour >= 11 && hour <= 21) || ringOn == false)
    {
      Serial.println("stopping alarm");
      vTaskDelay(pdMS_TO_TICKS(5 * 60 * 1000));
      sendOffPostRequest();
      ringing = false;
    }
    vTaskDelete(Alarm);
  }
}

void ringAlarm(void *parameter)
{
  unsigned long startTime = millis() + 180000;
  unsigned long currentTime = millis();

  unsigned long previousMillisLowBrightness = 0;
  unsigned long previousMillisMaxBrightness = 0;
  const long intervalBrightness = 3000;

  bool ringOn = alarms[weekday() - 1].soundOn;

  int currentHour = hour();
  int currentMinute = minute();

  int alarmMelody[] = {NOTE_A5, NOTE_A5, NOTE_A6, NOTE_A6, NOTE_A6, NOTE_A6, NOTE_A4, NOTE_A4};
  int alarmDurations[] = {4, 4, 4, 7, 7, 7, 4, 4};

  if (!(currentHour >= 11 && currentHour <= 21) || ringOn == false)
  {
    sendOnPostRequest();
  }

  while (true)
  {
    Serial.println("ringin Alarm");
    currentTime = millis();

    if ((currentTime >= startTime || WiFi.SSID() != "dragonn2" || WiFi.status() != WL_CONNECTED || (currentHour >= 11 && currentHour <= 21)) && ringOn == true)
    {

      for (int i = 0; i < sizeof(alarmMelody) / sizeof(alarmMelody[0]); i++)
      {
        tone(BUZZER_PIN, alarmMelody[i], 1000 / alarmDurations[i]);
        delay(1000 / alarmDurations[i] * 1.30);
        noTone(BUZZER_PIN);

        touchStopAlarm(currentHour, ringOn);
      }
      vTaskDelay(100);
    }
    touchStopAlarm(currentHour, ringOn);

    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void sendOnPostRequest()
{
  if ((WiFi.status() == WL_CONNECTED))
  { // Check WiFi connection status
    if (WiFi.SSID() == SSID1)
    {

      HTTPClient http;

      http.begin(LIGHT_IP);                               // Specify destination for HTTP request
      http.addHeader("Content-Type", "application/json"); // Specify content-type header

      int httpResponseCode = http.POST("{\"state\": \"ON\", \"transition\": 300}"); // Send the actual POST request

      if (httpResponseCode > 0)
      {
        String response = http.getString(); // Get the response to the request
        Serial.println(httpResponseCode);   // Print return code
        Serial.println(response);           // Print request answer
      }
      else
      {
        Serial.print("Error on sending POST: ");
        Serial.println(httpResponseCode);
      }

      http.end(); // Free resources
    }
  }
}

void sendOffPostRequest()
{
  if ((WiFi.status() == WL_CONNECTED))
  { // Check WiFi connection status
    if (WiFi.SSID() == SSID1)
    {
      HTTPClient http;

      http.begin("http://192.168.88.74/gateways/4276/RGB/command"); // Specify destination for HTTP request
      http.addHeader("Content-Type", "application/json");           // Specify content-type header

      int httpResponseCode = http.POST("{\"state\": \"OFF\", \"transition\": 30}"); // Send the actual POST request

      if (httpResponseCode > 0)
      {
        String response = http.getString(); // Get the response to the request
        Serial.println(httpResponseCode);   // Print return code
        Serial.println(response);           // Print request answer
      }
      else
      {
        Serial.print("Error on sending POST: ");
        Serial.println(httpResponseCode);
      }

      http.end(); // Free resources
    }
  }
}