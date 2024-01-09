#include "alarms.h"
#include <HTTPClient.h>

Day days[] = {
    {false, 0, 0},  // Sunday
    {false, 0, 0},  // Monday
    {false, 0, 0},  // Tuesday
    {false, 0, 0},  // Wednesday
    {false, 0, 0},  // Thursday
    {false, 0, 0},  // Friday
    {false, 0, 0}   // Saturday
};

bool ringedToday = false;

void ringAlarm(void *parameter);
void createRiningingTask();

void sendOnPostRequest();
void sendOffPostRequest();

void checkAlarm(int alarmHours, int alarmMinutes);
void checkAllAlarms(void *pvParameters);


void createAlarmTask() {
    xTaskCreatePinnedToCore(
    checkAllAlarms,          // Function to implement the task
    "Alarms",        // Task name
    10000,           // Stack size (words)
    NULL,            // Task input parameter
    1,               // Priority (0 is lowest)
    NULL,   // Task handle
    0                // Core to run the task on (0 or 1)
  );
}

void checkAllAlarms(void *pvParameters) {
    while(true) {
        int currentDay = weekday() - 1; // Adjust to 0-based index

        if (days[currentDay].isSet) {
            checkAlarm(days[currentDay].hours, days[currentDay].minutes);
        }

        if (hour() == 0 && minute() == 0 && second() == 0) {
            ringedToday = false;
        }

    vTaskDelay(pdMS_TO_TICKS(1 * 1000));
    }
}




void checkAlarm(int alarmHours, int alarmMinutes) {
  // Get the current time
  int currentHours = hour();
  int currentMinutes = minute();

  // Check if it's time for the alarm
  if (currentHours == alarmHours && currentMinutes == alarmMinutes && ringedToday == false) {
    Serial.print("Alarm! It's time to wake up on ");
    ringedToday = true;
    createRiningingTask(); 
    }
}


TaskHandle_t Alarm;

void createRiningingTask() {
  xTaskCreatePinnedToCore(
      ringAlarm,   // Function to implement the task
      "ringAlarm", // Name of the task
      10000,          // Stack size (words)
      NULL,           // Parameter to pass
      2,              // Priority
      &Alarm,           // Task handle
      0               // Core to run the task on (Core 1)
  );
}

void ringAlarm(void *parameter) {
  unsigned long startTime = millis() + 180000;
  unsigned long currentTime = millis();
  sendOnPostRequest();
  while(true)
  {
    currentTime = millis();
  if (currentTime >= startTime || WiFi.SSID() != "dragonn2" || WiFi.status() != WL_CONNECTED) {

    for (int dutyCycle = 0; dutyCycle <= 255; dutyCycle++) {
      ledcWrite(0, dutyCycle);
      Serial.println(dutyCycle);
    }
    for (int dutyCycle = 255; dutyCycle >= 0; dutyCycle--) {
      ledcWrite(0, dutyCycle);
      Serial.println(dutyCycle);
    }
  }

  if (touchRead(TOUCH_BUTTON_PIN) < TOUCH_BUTTON_THRESHOLD)
  {
    vTaskDelay(pdMS_TO_TICKS(5 * 60 * 1000));
    sendOffPostRequest();
    vTaskDelete(Alarm);
  }
}
}




void sendOnPostRequest() {
 if ((WiFi.status() == WL_CONNECTED)) { //Check WiFi connection status
 if (WiFi.SSID() == "dragonn2")
 {
 
  HTTPClient http;

  http.begin("http://192.168.88.74/gateways/0x1/RGB/command"); //Specify destination for HTTP request
  http.addHeader("Content-Type", "application/json"); //Specify content-type header

  int httpResponseCode = http.POST("{\"state\": \"ON\", \"transition\": 300}"); //Send the actual POST request

  if (httpResponseCode > 0) {
    String response = http.getString();                   //Get the response to the request
    Serial.println(httpResponseCode); //Print return code
    Serial.println(response);         //Print request answer
  } else {
    Serial.print("Error on sending POST: ");
    Serial.println(httpResponseCode);
  }

  http.end(); //Free resources
 }
 }
}




void sendOffPostRequest() {
 if ((WiFi.status() == WL_CONNECTED)) { //Check WiFi connection status
  if (WiFi.SSID() == "dragonn2") {
  HTTPClient http;

  http.begin("http://192.168.88.74/gateways/0x1/RGB/command"); //Specify destination for HTTP request
  http.addHeader("Content-Type", "application/json"); //Specify content-type header

  int httpResponseCode = http.POST("{\"state\": \"OFF\", \"transition\": 300}"); //Send the actual POST request

  if (httpResponseCode > 0) {
    String response = http.getString();                   //Get the response to the request
    Serial.println(httpResponseCode); //Print return code
    Serial.println(response);         //Print request answer
  } else {
    Serial.print("Error on sending POST: ");
    Serial.println(httpResponseCode);
  }

  http.end(); //Free resources
 }
 }
}