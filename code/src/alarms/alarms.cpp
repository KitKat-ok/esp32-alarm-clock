#include "alarms.h"
#include <ArduinoJson.h>

Alarm alarms[MAX_ALARMS];

TaskHandle_t alarmTaskHandle;
TaskHandle_t Alarm;

extern TaskHandle_t menuTaskHandle;

bool initialLightWasOn = false;
enum LightUserChoice
{
  CHOICE_PENDING,
  CHOICE_TURN_OFF_NOW,
  CHOICE_KEEP_PREVIOUS,
  CHOICE_DEFAULT
};
LightUserChoice userLightChoice = CHOICE_PENDING;

void ringAlarm(void *parameter);
void createRiningingTask();

void sendOnPostRequest(bool instant = false);
void sendOffPostRequest(bool instant = false);
void sendTogglePostRequest();
bool fetchInitialLightState();

void httpOnTask(void *param)
{
  if (WiFi.status() == WL_CONNECTED)
  {
    sendOnPostRequest(false);
  }
  vTaskDelete(NULL);
}

void httpOffTask(void *param)
{
  vTaskDelay(pdMS_TO_TICKS(5 * 60 * 1000));
  if (WiFi.status() == WL_CONNECTED)
  {
    sendOffPostRequest(false);
  }
  vTaskDelete(NULL);
}

void httpToggleTask(void *param)
{
  if (WiFi.status() == WL_CONNECTED)
  {
    sendTogglePostRequest();
  }
  vTaskDelete(NULL);
}

void checkAlarm(int index);
void checkAlarmsTask(void *pvParameters);

void initialzeAlarmArray()
{
  for (int i = 0; i < MAX_ALARMS; i++)
  {
    alarms[i] = {
        false,
        false,
        {true, true, true, true, true, true, true},
        0,
        0,
        true,
        true
    };
  }
}

void createAlarmTask()
{
  xTaskCreate(
      checkAlarmsTask,
      "Alarms",
      2048,
      NULL,
      1,
      &alarmTaskHandle
  );
}

void checkAlarms()
{
  int currentDay = weekday() - 1;
  Serial.println("current day:" + String(currentDay));

  for (int i = 0; i < MAX_ALARMS; ++i)
  {
    if (alarms[i].days[currentDay] && alarms[i].enabled && alarms[i].exists)
    {
      checkAlarm(i);
    }
  }
}

void checkAlarmsTask(void *pvParameters)
{
  int previousHour = hour();
  int previousMinute = minute();
  while (true)
  {
    int s = second();
    if (s != 0)
    {
      vTaskDelay(pdMS_TO_TICKS((60 - s) * 1000));
    }
    int currentHour = hour();
    int currentMinute = minute();

    if (currentHour != previousHour || currentMinute != previousMinute)
    {
      previousHour = currentHour;
      previousMinute = currentMinute;
      checkAlarms();
    }
    vTaskDelay(pdMS_TO_TICKS(60000));
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
    alarms[i].enabled = true;
  }
}

int lastRingingMinute = -1;
int lastRingingHour = -1;
bool ringing = false;

bool buzzerEnabled = false;
bool lightCtrlEnabled = false;

void checkAlarm(int index)
{
  int currentHours = hour();
  int currentMinutes = minute();
  Serial.println("Check Alarm Function");

  if (currentHours == alarms[index].hours && currentMinutes == alarms[index].minutes && (ringing == false || lastRingingMinute != currentMinutes || lastRingingHour != currentHours))
  {
    Serial.print("Alarm! It's time to wake up on ");
    buzzerEnabled = alarms[index].soundOn;
    lightCtrlEnabled = alarms[index].lightOn;
    createRiningingTask();

    lastRingingMinute = currentMinutes;
    lastRingingHour = currentHours;
    ringing = true;
  }
}

String formatTimeUntilNextAlarm()
{
  long remainingSeconds = getTimeUntilNextAlarm();

  if (remainingSeconds < 0)
  {
    return "No Alarms";
  }

  long days = remainingSeconds / 86400L;
  long hours = (remainingSeconds % 86400L) / 3600L;
  long minutes = (remainingSeconds % 3600L) / 60L;

  String result = "Alarm: ";

  if (days > 0)
  {
    result += String(days) + "d " + String(hours) + "h";
  }
  else if (hours > 0)
  {
    result += String(hours) + "h " + String(minutes) + "m";
  }
  else
  {
    result += String(minutes) + "m";
  }

  return result;
}

long getTimeUntilNextAlarm()
{
  int currentDay = weekday() - 1;
  int currentH = hour();
  int currentM = minute();
  int currentS = second();

  long nowInSeconds = ((long)currentDay * 86400L) + ((long)currentH * 3600L) + ((long)currentM * 60L) + currentS;

  long minRemainingSeconds = -1;

  for (int i = 0; i < MAX_ALARMS; i++)
  {
    if (!alarms[i].exists || !alarms[i].enabled)
    {
      continue;
    }

    for (int dayOffset = 0; dayOffset < 7; dayOffset++)
    {
      int targetDay = (currentDay + dayOffset) % 7;

      if (alarms[i].days[targetDay])
      {
        long alarmInSeconds = ((long)targetDay * 86400L) + ((long)alarms[i].hours * 3600L) + ((long)alarms[i].minutes * 60L);

        if (alarmInSeconds <= nowInSeconds)
        {
          alarmInSeconds += (7L * 86400L);
        }

        long diff = alarmInSeconds - nowInSeconds;

        if (minRemainingSeconds == -1 || diff < minRemainingSeconds)
        {
          minRemainingSeconds = diff;
        }
      }
    }
  }

  return minRemainingSeconds;
}

void createRiningingTask()
{
  xTaskCreate(
      ringAlarm,
      "ringAlarm",
      4096,
      NULL,
      4,
      &Alarm
  );
}

struct Melody
{
  const int *notes;
  const int *durations;
  int length;
};

void playMelodyStep(const Melody &m, int &noteIdx, unsigned long &noteStartTime, bool &isPlaying)
{
  if (noteIdx >= m.length)
  {
    noteIdx = 0;
  }

  int duration = 1000 / m.durations[noteIdx];
  int pauseBetweenNotes = duration * 0.3;

  if (!isPlaying)
  {
    tone(BUZZER_PIN, m.notes[noteIdx], duration);
    noteStartTime = millis();
    isPlaying = true;
  }
  else if (millis() - noteStartTime >= (unsigned long)(duration + pauseBetweenNotes))
  {
    noTone(BUZZER_PIN);
    isPlaying = false;
    noteIdx++;

    if (noteIdx >= m.length)
    {
      noteIdx = 0;
    }
  }
}

void showRingingAlarmScreen()
{
  oled.clearDisplay();

  oled.setFont(&DejaVu_LGC_Sans_Bold_9);
  oled.setTextColor(SSD1327_WHITE);
  
  String dateStr = String(day()) + "." + String(month()) + "." + String(year());
  centerText(dateStr, 11);

  oled.drawLine(0, 16, 127, 16, 6);

  bool showColon = (millis() / 500) % 2 == 0;
  String timeStr = formatWithLeadingZero(hour()) + (showColon ? ":" : " ") + formatWithLeadingZero(minute());

  oled.setFont(&DejaVu_Sans_Bold_16);
  oled.setTextColor(SSD1327_WHITE);
  centerText(timeStr, 44);

  oled.setFont(&DejaVu_LGC_Sans_Bold_9);
  oled.setTextColor(12);
  centerText(getCurrentWeekdayName(), 60);

  oled.drawLine(0, 66, 127, 66, 6);

  bool isBlinking = (millis() / 500) % 2 == 0;

  oled.setFont(&DejaVu_LGC_Sans_Bold_10);

  if (isBlinking)
  {
    oled.fillRect(4, 70, 120, 18, 15);
    oled.setTextColor(0);
  }
  else
  {
    oled.drawRect(4, 70, 120, 18, 6);
    oled.setTextColor(SSD1327_WHITE);
  }

  centerText("ALARM RINGING!", 83);

  oled.drawLine(0, 96, 127, 96, 6);

  oled.setFont(&Roboto_Black_9);
  oled.setTextColor(12);
  centerText("Press Any Button / Touch", 108);

  oled.setTextColor(SSD1327_WHITE);
  centerText("or Hall Sensor to STOP", 121);

  delay(10);
  oledMana.display();

  oled.setFont(&DejaVu_LGC_Sans_Bold_10);
  oled.setTextColor(SSD1327_WHITE);
}

void showLightOptionScreen(int selectedOption)
{
  oled.clearDisplay();

  oled.setFont(&DejaVu_LGC_Sans_Bold_9);
  oled.setTextColor(SSD1327_WHITE);
  centerText("LIGHT SETTINGS", 12);

  oled.drawLine(0, 16, 127, 16, 6);

  const char *options[] = {"Turn OFF Light", "Keep Prev State"};
  const int totalOptions = 2;
  const int boxHeight = 26;
  const int startY = 22;
  const int ySpacing = 32;

  oled.setFont(&Roboto_Black_9);

  for (int i = 0; i < totalOptions; i++)
  {
    int boxY = startY + (i * ySpacing);
    bool isSelected = (selectedOption == i);

    if (isSelected)
    {
      oled.fillRect(4, boxY, 120, boxHeight, 15);
      oled.setTextColor(0);
      centerText("> " + String(options[i]) + " <", boxY + 17);
    }
    else
    {
      oled.drawRect(4, boxY, 120, boxHeight, 3);
      oled.setTextColor(10);
      centerText(options[i], boxY + 17);
    }
  }

  oled.drawLine(0, 90, 127, 90, 6);

  oled.setFont(&Roboto_Black_9);
  oled.setTextColor(12);
  centerText("Up/Down: Switch | Menu: OK", 104);

  oled.setTextColor(SSD1327_WHITE);
  centerText("Touch/Back: Default Off", 118);

  delay(10);
  oledMana.display();

  oled.setFont(&DejaVu_LGC_Sans_Bold_10);
  oled.setTextColor(SSD1327_WHITE);
}

void ringAlarm(void *parameter)
{
  unsigned long startTime = millis();
  bool ringOn = buzzerEnabled;
  bool lightOn = lightCtrlEnabled;
  bool isWifiAvailable = (WiFi.status() == WL_CONNECTED);

  if (menuTaskHandle != NULL)
  {
    vTaskSuspend(menuTaskHandle);
  }

  if (lightOn && isWifiAvailable)
  {
    initialLightWasOn = fetchInitialLightState();
    xTaskCreate(httpOnTask, "httpOnTask", 4096, NULL, 1, NULL);
  }

  int alarmMelody[] = {NOTE_C5, NOTE_C5, NOTE_B4, NOTE_B4, NOTE_B4, NOTE_B4, NOTE_G4, NOTE_G4};
  int alarmDurations[] = {8, 8, 8, 12, 12, 12, 8, 8};

  int SecAlarmMelody[] = {NOTE_C3, NOTE_C3, NOTE_B2, NOTE_B2, NOTE_B2, NOTE_B2, NOTE_G2, NOTE_G2};
  int SecAlarmDurations[] = {4, 4, 4, 8, 8, 8, 4, 4};

  int QuietAlarmMelody[] = {NOTE_C4, NOTE_C4, NOTE_B3, NOTE_B3, NOTE_G3, NOTE_G3};
  int QuietAlarmDurations[] = {16, 16, 16, 16, 16, 16};

  Melody melody1 = {alarmMelody, alarmDurations, (int)(sizeof(alarmMelody) / sizeof(alarmMelody[0]))};
  Melody melody2 = {SecAlarmMelody, SecAlarmDurations, (int)(sizeof(SecAlarmMelody) / sizeof(SecAlarmMelody[0]))};
  Melody quietMelody = {QuietAlarmMelody, QuietAlarmDurations, (int)(sizeof(QuietAlarmMelody) / sizeof(QuietAlarmMelody[0]))};

  Serial.println("Starting Alarm");
  unsigned long lastLedToggle = startTime;
  unsigned long lastToggleRequestTime = startTime;
  unsigned long lastDisplayTime = 0;
  bool stopAlarmRequested = false;
  unsigned long stopTimerStart = 0;

  bool lastInputState = false;

  userLightChoice = CHOICE_DEFAULT;
  int currentMenuOption = 0; 

  int currentNote = 0;
  unsigned long noteStartTime = 0;
  bool notePlaying = false;
  int activeMelodyType = -1;

  unsigned long inputGracePeriod = millis();

  while (true)
  {
    inputDetected = true;

    inkButtonStates btn = useButton();
    touchState touch = useTouch();
    bool hallTriggered = readHallSwitch();

    bool anyButtonPress = (btn != None) || touch.longPress || hallTriggered;
    bool touchTap = touch.touched && !touch.longPress;

    if (millis() - lastDisplayTime >= 100)
    {
      if (!stopAlarmRequested)
      {
        showRingingAlarmScreen();
      }
      else if (lightOn && isWifiAvailable)
      {
        showLightOptionScreen(currentMenuOption);
      }
      lastDisplayTime = millis();
    }

    if (ringOn && !stopAlarmRequested)
    {
      int currentHour = hour();
      int selectedType = 0;

      if (currentHour >= 11 && currentHour <= 21)
      {
        selectedType = 0;
      }
      else if (!touch.touched)
      {
        selectedType = 1; 
      }
      else
      {
        selectedType = 2;
      }

      if (activeMelodyType != selectedType)
      {
        activeMelodyType = selectedType;
        currentNote = 0;
        notePlaying = false;
        noTone(BUZZER_PIN);
      }

      if (activeMelodyType == 0)
      {
        playMelodyStep(quietMelody, currentNote, noteStartTime, notePlaying);
      }
      else if (activeMelodyType == 1)
      {
        playMelodyStep(melody1, currentNote, noteStartTime, notePlaying);
      }
      else
      {
        playMelodyStep(melody2, currentNote, noteStartTime, notePlaying);
      }
    }

    if (!stopAlarmRequested && (millis() - startTime >= 30 * 60 * 1000))
    {
      Serial.println("30 minutes elapsed with no input. Exiting alarm task...");
      userLightChoice = CHOICE_DEFAULT;
      break;
    }

    if (millis() - inputGracePeriod > 1000)
    {
      if (!stopAlarmRequested)
      {
        if ((anyButtonPress || touchTap) && !lastInputState)
        {
          stopAlarmRequested = true;
          stopTimerStart = millis();
          noTone(BUZZER_PIN);

          if (!lightOn || !isWifiAvailable)
          {
            Serial.println("Alarm silenced. No Wi-Fi or light ctrl disabled; skipping light menu.");
            break;
          }

          lastInputState = true;
          Serial.println("Alarm silenced. Opening Light Selection Menu...");
          vTaskDelay(pdMS_TO_TICKS(500));
          continue;
        }
        lastInputState = anyButtonPress || touchTap;
      }
      else
      {
        if ((btn == Up || btn == LongUp) && !lastInputState)
        {
          currentMenuOption = 0;
          Serial.println("Selected: Turn OFF Light");
        }
        else if ((btn == Down || btn == LongDown) && !lastInputState)
        {
          currentMenuOption = 1;
          Serial.println("Selected: Keep Prev State");
        }
        else if ((btn == Menu || btn == LongMenu || hallTriggered) && !lastInputState)
        {
          userLightChoice = (currentMenuOption == 0) ? CHOICE_TURN_OFF_NOW : CHOICE_KEEP_PREVIOUS;
          Serial.println("Choice confirmed via Menu button! Exiting...");
          break;
        }
        else if ((btn == Back || btn == LongBack) && !lastInputState)
        {
          userLightChoice = CHOICE_DEFAULT;
          Serial.println("Back pressed. Exiting with default light behavior...");
          break;
        }
        else if (touchTap && !lastInputState)
        {
          userLightChoice = CHOICE_DEFAULT;
          Serial.println("Touch pressed. Exiting with default light behavior...");
          break;
        }

        lastInputState = anyButtonPress || touchTap;
      }
    }

    if (stopAlarmRequested && (millis() - stopTimerStart >= 120000))
    {
      Serial.println("2 minutes timeout elapsed without selection. Using default behavior.");
      userLightChoice = CHOICE_DEFAULT;
      break;
    }

    if (!stopAlarmRequested && isWifiAvailable && (millis() - startTime >= 30000))
    {
      if (millis() - lastToggleRequestTime >= 10000)
      {
        xTaskCreate(httpToggleTask, "httpToggleTask", 4096, NULL, 1, NULL);
        lastToggleRequestTime = millis();
      }
    }

    if (millis() - lastLedToggle >= 2000)
    {
      toggleLeds();
      lastLedToggle = millis();
    }

    vTaskDelay(pdMS_TO_TICKS(30));
  }

  turnOffLeds();
  noTone(BUZZER_PIN);

  if (lightOn && WiFi.status() == WL_CONNECTED)
  {
    if (userLightChoice == CHOICE_TURN_OFF_NOW)
    {
      sendOffPostRequest(true);
    }
    else if (userLightChoice == CHOICE_KEEP_PREVIOUS)
    {
      if (initialLightWasOn)
      {
        sendOnPostRequest(true);
      }
      else
      {
        sendOffPostRequest(true);
      }
    }
    else
    {
      xTaskCreate(httpOffTask, "httpOffTask", 4096, NULL, 1, NULL);
    }
  }

  ringing = false;

  if (menuTaskHandle != NULL)
  {
    vTaskResume(menuTaskHandle);
  }

  vTaskDelete(NULL);
}

bool fetchInitialLightState()
{
  if (WiFi.status() == WL_CONNECTED && WiFi.SSID() == SSID1)
  {
    HTTPClient http;
    String url = "http://" + String(LIGHT_IP) + "/gateways/4276/RGB/";
    http.begin(url);

    int httpResponseCode = http.GET();
    if (httpResponseCode > 0)
    {
      String payload = http.getString();

      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, payload);

      if (!error && doc["state"].is<String>())
      {
        String state = doc["state"].as<String>();
        http.end();
        return (state == "ON");
      }
    }
    http.end();
  }
  return false;
}

void sendOnPostRequest(bool instant)
{
  if ((WiFi.status() == WL_CONNECTED))
  {
    if (WiFi.SSID() == SSID1)
    {
      HTTPClient http;

      http.begin(LIGHT_GATE);
      http.addHeader("Content-Type", "application/json");

      int httpResponseCode = 0;
      if (instant)
      {
        httpResponseCode = http.POST("{\"state\": \"ON\"}");
      }
      else
      {
        httpResponseCode = http.POST("{\"state\": \"ON\", \"transition\": 300}");
      }

      if (httpResponseCode > 0)
      {
        String response = http.getString();
        Serial.println(httpResponseCode);
        Serial.println(response);
      }
      else
      {
        Serial.print("Error on sending POST: ");
        Serial.println(httpResponseCode);
      }

      http.end();
    }
  }
}

void sendOffPostRequest(bool instant)
{
  if ((WiFi.status() == WL_CONNECTED))
  {
    if (WiFi.SSID() == SSID1)
    {
      HTTPClient http;

      http.begin(LIGHT_GATE);
      http.addHeader("Content-Type", "application/json");

      int httpResponseCode = 0;
      if (instant)
      {
        httpResponseCode = http.POST("{\"state\": \"OFF\"}");
      }
      else
      {
        httpResponseCode = http.POST("{\"state\": \"OFF\", \"transition\": 30}");
      }

      if (httpResponseCode > 0)
      {
        String response = http.getString();
        Serial.println(httpResponseCode);
        Serial.println(response);
      }
      else
      {
        Serial.print("Error on sending POST: ");
        Serial.println(httpResponseCode);
      }

      http.end();
    }
  }
}

void sendTogglePostRequest()
{
  if ((WiFi.status() == WL_CONNECTED))
  {
    if (WiFi.SSID() == SSID1)
    {
      HTTPClient http;

      http.begin(LIGHT_GATE);
      http.addHeader("Content-Type", "application/json");

      int httpResponseCode = http.POST("{\"command\": \"toggle\"}");

      if (httpResponseCode > 0)
      {
        String response = http.getString();
        Serial.println(httpResponseCode);
        Serial.println(response);
      }
      else
      {
        Serial.print("Error on sending POST: ");
        Serial.println(httpResponseCode);
      }

      http.end();
    }
  }
}