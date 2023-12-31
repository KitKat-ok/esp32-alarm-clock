#include "alarms.h"


bool sunday = false;
bool monday = false;
bool tuesday = false;
bool wednesday = false;
bool thursday = false;
bool friday = false;
bool saturday = false;

int hoursSunday = 0;
int minutesSunday = 0;
int hoursMonday = 0;
int minutesMonday = 0;
int hoursTuesday = 0;
int minutesTuesday = 0;
int hoursWednesday = 0;
int minutesWednesday = 0;
int hoursThursday = 0;
int minutesThursday = 0;
int hoursFriday = 0;
int minutesFriday = 0;
int hoursSaturday = 0;
int minutesSaturday = 0;

bool ringedToday = false;

void ringAlarm();

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
  while(1) {
  switch (weekday()) {
    case 1:
      Serial.println("Sunday");
      if (sunday = true)
      {
          checkAlarm(hoursSunday, minutesSunday);
      }
      break;
    case 2:
          Serial.println("Monday");
                if (monday = true)
      {
          checkAlarm(hoursMonday, minutesMonday);
      }
      break;
    case 3:
      Serial.println("Tuesday");
            if (tuesday = true)
      {
          checkAlarm(hoursTuesday, minutesTuesday);
      }
      break;
    case 4:
      Serial.println("Wednesday");
            if (wednesday = true)
      {
          checkAlarm(hoursWednesday, minutesWednesday);
      }
      break;
    case 5:
      Serial.println("Thursday");
            if (thursday = true)
      {
          checkAlarm(hoursThursday, minutesThursday);
      }
      break;
    case 6:
      Serial.println("Friday");
            if (friday = true)
      {
          checkAlarm(hoursFriday, minutesFriday);
      }
      break;
    case 7:
      Serial.println("Saturday");
            if (saturday = true)
      {
          checkAlarm(hoursSaturday, minutesSaturday);
      }
      break;
  }
    if (hour() == 0 && minute() == 0 && second() == 0)
    {
      ringedToday = false;
    }
    
    vTaskDelay(pdMS_TO_TICKS(1000));
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
    ringAlarm();  
    }
}


void ringAlarm() {
    for (int dutyCycle = 0; dutyCycle <= 255; dutyCycle++) {
    // Set the PWM duty cycle (0 to 255)
    ledcWrite(0, dutyCycle);
    delay(10);  // Adjust the delay based on your needs
  }

   for (int dutyCycle = 255; dutyCycle >= 0; dutyCycle--) {
    // Set the PWM duty cycle (0 to 255)
    ledcWrite(0, dutyCycle);
    delay(10);  // Adjust the delay based on your needs
  }
}