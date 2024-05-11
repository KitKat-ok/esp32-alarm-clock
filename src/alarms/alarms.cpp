#include "alarms.h"
#include <HTTPClient.h>

Alarm alarms[] = {
    {false, 0, 0, true}, // Sunday
    {false, 0, 0, true}, // Monday
    {false, 0, 0, true}, // Tuesday
    {false, 0, 0, true}, // Wednesday
    {false, 0, 0, true}, // Thursday
    {false, 0, 0, true}, // Friday
    {false, 0, 0, true}  // Saturday
};

bool ringedToday = false;

void ringAlarm(void *parameter);
void createRiningingTask();

void sendOnPostRequest();
void sendOffPostRequest();

void checkAlarm(int alarmHours, int alarmMinutes);
void checkAllAlarms(void *pvParameters);

void createAlarmTask()
{
  xTaskCreatePinnedToCore(
      checkAllAlarms, // Function to implement the task
      "Alarms",       // Task name
      10000,          // Stack size (words)
      NULL,           // Task input parameter
      1,              // Priority (0 is lowest)
      NULL,           // Task handle
      0               // Core to run the task on (0 or 1)
  );
}

void checkAllAlarms(void *pvParameters)
{
  while (true)
  {
    int currentDay = weekday() - 1; // Adjust to 0-based index

    if (alarms[currentDay].isSet == true && ringedToday == false)
    {
      Serial.println("checking alarm");
      checkAlarm(alarms[currentDay].hours, alarms[currentDay].minutes);
    }

    if (hour() == 0 && minute() == 0 && second() == 0)
    {
      ringedToday = false;
    }

    vTaskDelay(pdMS_TO_TICKS(1 * 1000));
  }
}

void disableAllAlarms()
{
  for (int i = 0; i < 7; i++)
  {
    alarms[i].isSet = false;
  }
}

void enableAllAlarms()
{
  for (int i = 0; i < 7; i++)
  {
    if (alarms[i].hours != 0 && alarms[i].minutes != 0)
    {
      alarms[i].isSet = true;
    }
  }
}

void checkAlarm(int alarmHours, int alarmMinutes)
{
  // Get the current time
  int currentHours = hour();
  int currentMinutes = minute();

  // Check if it's time for the alarm
  if (currentHours == alarmHours && currentMinutes == alarmMinutes && ringedToday == false)
  {
    Serial.print("Alarm! It's time to wake up on ");
    ringedToday = true;
    createRiningingTask();
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
  if (touchRead(TOUCH_BUTTON_PIN) < TOUCH_BUTTON_THRESHOLD)
  {
    if (!(currentHour >= 11 && currentHour <= 21) || ringOn == false)
    {
      vTaskDelay(pdMS_TO_TICKS(5 * 60 * 1000));
      sendOffPostRequest();
    }
    vTaskDelete(Alarm);
  }
}

void ringAlarm(void *parameter)
{
  unsigned long startTime = millis() + 180000;
  unsigned long currentTime = millis();

  unsigned long previousMillisBrightness = 0;
  const long intervalBrightness = 6000;

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
    display.ssd1306_command(SSD1306_DISPLAYON);
    currentTime = millis();
    if (currentTime - previousMillisBrightness >= intervalBrightness)
    {

      previousMillisBrightness = currentTime;

      display.ssd1306_command(SSD1306_DISPLAYON);

      display.dim(true);
      LedDisplay.setBrightness(0);
      LedDisplay.showNumberDecEx(currentHour * 100 + currentMinute, 0b11100000, true);
    }
    currentTime = millis();

    if ((currentTime >= startTime || WiFi.SSID() != "dragonn2" || WiFi.status() != WL_CONNECTED || !(currentHour >= 11 && currentHour <= 21)) && ringOn == true)
    {

      for (int i = 0; i < sizeof(alarmMelody) / sizeof(alarmMelody[0]); i++)
      {
        tone(BUZZER_PIN, alarmMelody[i], 1000 / alarmDurations[i]);
        delay(1000 / alarmDurations[i] * 1.30);
        noTone(BUZZER_PIN);

        touchStopAlarm(currentHour, ringOn);
      }
      vTaskDelay(100);
      vTaskDelay(1000);
    }

    currentTime = millis();
    if (currentTime - previousMillisBrightness >= intervalBrightness2)
    {

      previousMillisBrightness = currentTime;

      display.dim(false);
      LedDisplay.setBrightness(7);
      LedDisplay.showNumberDecEx(currentHour * 100 + currentMinute, 0b11100000, true);
    }

    touchStopAlarm(currentHour, ringOn);

    vTaskDelay(pdMS_TO_TICKS(1));
  }
}

void sendOnPostRequest()
{
  if ((WiFi.status() == WL_CONNECTED))
  { // Check WiFi connection status
    if (WiFi.SSID() == "dragonn2")
    {

      HTTPClient http;

      http.begin("http://192.168.88.74/gateways/0x1/RGB/command"); // Specify destination for HTTP request
      http.addHeader("Content-Type", "application/json");          // Specify content-type header

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
    if (WiFi.SSID() == "dragonn2")
    {
      HTTPClient http;

      http.begin("http://192.168.88.74/gateways/0x1/RGB/command"); // Specify destination for HTTP request
      http.addHeader("Content-Type", "application/json");          // Specify content-type header

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