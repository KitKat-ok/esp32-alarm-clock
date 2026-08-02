#include "alarms.h"
#include <ArduinoJson.h>

Alarm alarms[MAX_ALARMS];

TaskHandle_t alarmTaskHandle;
TaskHandle_t Alarm;

// External handle to the main UI/Menu task
extern TaskHandle_t menuTaskHandle;

// Global state variables for light status
bool initialLightWasOn = false;
enum LightUserChoice { CHOICE_PENDING, CHOICE_TURN_OFF_NOW, CHOICE_KEEP_PREVIOUS, CHOICE_DEFAULT };
LightUserChoice userLightChoice = CHOICE_PENDING;

void ringAlarm(void *parameter);
void createRiningingTask();

void sendOnPostRequest(bool instant = false);
void sendOffPostRequest(bool instant = false);
void sendTogglePostRequest();
bool fetchInitialLightState();

// Tasks for background HTTP requests to avoid blocking UI execution
void httpOnTask(void *param)
{
  sendOnPostRequest(false);
  vTaskDelete(NULL);
}

// Waits 5 minutes in the background before sending the standard OFF request
void httpOffTask(void *param)
{
  vTaskDelay(pdMS_TO_TICKS(5 * 60 * 1000));
  sendOffPostRequest(false);
  vTaskDelete(NULL);
}

void httpToggleTask(void *param)
{
  sendTogglePostRequest();
  vTaskDelete(NULL);
}

void checkAlarm(int index);
void checkAlarmsTask(void *pvParameters);

void initialzeAlarmArray()
{
  for (int i = 0; i < MAX_ALARMS; i++)
  {
    alarms[i] = {
        false,                                      // exists
        false,                                      // enabled
        {true, true, true, true, true, true, true}, // days
        0,                                          // hours
        0,                                          // minutes
        true,                                       // soundOn
        true                                        // lightOn
    };
  }
}

void createAlarmTask()
{
  xTaskCreate(
      checkAlarmsTask, // Function to implement the task
      "Alarms",        // Task name
      2048,            // Stack size (words)
      NULL,            // Task input parameter
      1,               // Priority (0 is lowest)
      &alarmTaskHandle // Task handle
  );
}

void checkAlarms()
{
  int currentDay = weekday() - 1; // Adjust to 0-based index
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
      vTaskDelay(pdMS_TO_TICKS((60 - s) * 1000));
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

void createRiningingTask()
{
  xTaskCreate(
      ringAlarm,   // Function to implement the task
      "ringAlarm", // Name of the task
      4096,        // Stack size (words)
      NULL,        // Parameter to pass
      4,           // Priority
      &Alarm       // Task handle
  );
}

struct Melody
{
  const int *notes;
  const int *durations;
  int length;
};

// NON-BLOCKING MELODY STEPPER (State Machine)
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
      noteIdx = 0; // Loop melody continuously
    }
  }
}

void showRingingAlarmScreen()
{
  oled.clearDisplay();

  // 1. Top Section: Date Header (0 - 16px)
  oled.setFont(&DejaVu_LGC_Sans_Bold_9);
  oled.setTextColor(SSD1327_WHITE);
  centerText(String(day()) + "." + String(month()) + "." + String(year()), 11);

  oled.drawLine(0, 16, 127, 16, 6);

  // 2. Large Time Section (20 - 58px)
  bool showColon = (millis() / 500) % 2 == 0;

  String timeStr = formatWithLeadingZero(hour()) + (showColon ? ":" : " ") + formatWithLeadingZero(minute());

  oled.setFont(&DejaVu_Sans_Bold_16);
  centerText(timeStr, 44);

  // Weekday Subtitle
  oled.setFont(&DejaVu_LGC_Sans_Bold_9);
  oled.setTextColor(12);
  centerText(getCurrentWeekdayName(), 60);

  oled.drawLine(0, 66, 127, 66, 6);

  // 3. Middle Section: Dynamic Status Banner (68 - 92px)
  oled.setFont(&DejaVu_LGC_Sans_Bold_10);
  oled.setTextColor(SSD1327_WHITE);
  centerText("ALARM RINGING!", 82);

  if ((millis() / 500) % 2 == 0)
  {
    oled.fillRect(4, 70, 120, 16, SSD1327_WHITE);
    oled.setTextColor(SSD1327_BLACK, SSD1327_WHITE);
    centerText("ALARM RINGING!", 82);
  }

  // 4. Bottom Footer: Stop Instructions (96 - 128px)
  oled.drawLine(0, 96, 127, 96, 6);

  oled.setFont(&Roboto_Black_9);
  oled.setTextColor(12);
  centerText("Press Any Button / Touch", 108);

  oled.setTextColor(SSD1327_WHITE);
  centerText("or Hall Sensor to STOP", 121);

  oledMana.display();
}

void showLightOptionScreen(int selectedOption)
{
  oled.clearDisplay();

  oled.setFont(&DejaVu_LGC_Sans_Bold_9);
  oled.setTextColor(SSD1327_WHITE);
  centerText("LIGHT SETTINGS", 12);
  oled.drawLine(0, 16, 127, 16, 6);

  oled.setFont(&Roboto_Black_9);

  // Option 0: Turn OFF Light
  if (selectedOption == 0)
  {
    oled.fillRect(4, 22, 120, 28, SSD1327_WHITE);
    oled.setTextColor(SSD1327_BLACK, SSD1327_WHITE);
    centerText("> Turn OFF Light <", 39);
  }
  else
  {
    oled.setTextColor(SSD1327_WHITE);
    centerText("Turn OFF Light", 39);
  }

  // Option 1: Keep Previous State
  if (selectedOption == 1)
  {
    oled.fillRect(4, 56, 120, 28, SSD1327_WHITE);
    oled.setTextColor(SSD1327_BLACK, SSD1327_WHITE);
    centerText("> Keep Prev State <", 73);
  }
  else
  {
    oled.setTextColor(SSD1327_WHITE);
    centerText("Keep Prev State", 73);
  }

  oled.drawLine(0, 90, 127, 90, 6);
  oled.setFont(&Roboto_Black_9);
  oled.setTextColor(12);
  centerText("Up/Down: Switch | Menu: OK", 104);
  oled.setTextColor(SSD1327_WHITE);
  centerText("Touch/Back: Default Off", 118);

  oledMana.display();
}

void ringAlarm(void *parameter)
{
  unsigned long startTime = millis();
  bool ringOn = buzzerEnabled;
  bool lightOn = lightCtrlEnabled;

  // Suspend main menu UI task
  if (menuTaskHandle != NULL)
  {
    vTaskSuspend(menuTaskHandle);
  }

  // Fetch initial state of the light at start of alarm
  if (lightOn && WiFi.status() == WL_CONNECTED)
  {
    initialLightWasOn = fetchInitialLightState();
    xTaskCreate(httpOnTask, "httpOnTask", 4096, NULL, 1, NULL);
  }

  // Melodies
  int alarmMelody[] = {NOTE_C5, NOTE_C5, NOTE_B4, NOTE_B4, NOTE_B4, NOTE_B4, NOTE_G4, NOTE_G4};
  int alarmDurations[] = {8, 8, 8, 12, 12, 12, 8, 8};

  int SecAlarmMelody[] = {NOTE_C3, NOTE_C3, NOTE_B2, NOTE_B2, NOTE_B2, NOTE_B2, NOTE_G2, NOTE_G2};
  int SecAlarmDurations[] = {4, 4, 4, 8, 8, 8, 4, 4};

  // Softer octave melody for daytime hours (11:00 to 21:00)
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
  int currentMenuOption = 0; // 0 = Turn OFF, 1 = Keep Prev State

  // Non-blocking melody playback states
  int currentNote = 0;
  unsigned long noteStartTime = 0;
  bool notePlaying = false;
  int activeMelodyType = -1;

  // Initial input grace period (1 second)
  unsigned long inputGracePeriod = millis();

  while (true)
  {
    inputDetected = true;

    // --- Single Read Hardware Inputs Per Frame ---
    inkButtonStates btn = useButton();
    touchState touch = useTouch();
    bool hallTriggered = readHallSwitch();

    bool anyButtonPress = (btn != None) || touch.longPress || hallTriggered;
    bool touchTap = touch.touched && !touch.longPress;

    // --- Render Alarm UI Screen (~10 FPS to save CPU) ---
    if (millis() - lastDisplayTime >= 100)
    {
      if (!stopAlarmRequested)
      {
        showRingingAlarmScreen();
      }
      else
      {
        showLightOptionScreen(currentMenuOption);
      }
      lastDisplayTime = millis();
    }

    // --- Play Melody Non-blocking ---
    if (ringOn && !stopAlarmRequested)
    {
      int currentHour = hour();
      int selectedType = 0;

      if (currentHour >= 11 && currentHour <= 21)
      {
        selectedType = 0; // Quiet Daytime Melody
      }
      else if (!touch.touched)
      {
        selectedType = 1; // Standard Melody
      }
      else
      {
        selectedType = 2; // Alternate Melody when touched
      }

      // Reset note index ONLY when switching melodies mid-song
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

    // --- 30-Minute Continuous Ringing Timeout Check ---
    if (!stopAlarmRequested && (millis() - startTime >= 30 * 60 * 1000))
    {
      Serial.println("30 minutes elapsed with no input. Silencing alarm...");
      stopAlarmRequested = true;
      stopTimerStart = millis();
      noTone(BUZZER_PIN);
    }

    // --- Process Input Logic ---
    if (millis() - inputGracePeriod > 1000)
    {
      if (!stopAlarmRequested)
      {
        // Any initial press silences the alarm and opens the option menu
        if ((anyButtonPress || touchTap) && !lastInputState)
        {
          stopAlarmRequested = true;
          stopTimerStart = millis();
          noTone(BUZZER_PIN);
          
          lastInputState = true;
          Serial.println("Alarm silenced. Opening Light Selection Menu...");
          vTaskDelay(pdMS_TO_TICKS(500)); // Longer debounce buffer so initial press isn't re-used
          continue;
        }
        lastInputState = anyButtonPress || touchTap;
      }
      else
      {
        // --- Light Option Menu State ---

        // Up / Down buttons switch menu selection
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

        // Menu / LongMenu confirms selected option
        else if ((btn == Menu || btn == LongMenu || hallTriggered) && !lastInputState)
        {
          userLightChoice = (currentMenuOption == 0) ? CHOICE_TURN_OFF_NOW : CHOICE_KEEP_PREVIOUS;
          Serial.println("Choice confirmed via Menu button! Exiting...");
          break;
        }

        // Back / LongBack triggers default behavior immediately
        else if ((btn == Back || btn == LongBack) && !lastInputState)
        {
          userLightChoice = CHOICE_DEFAULT;
          Serial.println("Back pressed. Exiting with default light behavior...");
          break;
        }

        // Touching the touch sensor automatically triggers default behavior
        else if (touchTap && !lastInputState)
        {
          userLightChoice = CHOICE_DEFAULT;
          Serial.println("Touch pressed. Exiting with default light behavior...");
          break;
        }

        lastInputState = anyButtonPress || touchTap;
      }
    }

    // 2-minute menu timeout -> defaults to original slow background behavior
    if (stopAlarmRequested && (millis() - stopTimerStart >= 120000))
    {
      Serial.println("2 minutes timeout elapsed without selection. Using default behavior.");
      userLightChoice = CHOICE_DEFAULT;
      break;
    }

    // Background light toggle request offloaded to background task
    if (!stopAlarmRequested && millis() - startTime >= 30000)
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

    // Yield CPU time (30 ms)
    vTaskDelay(pdMS_TO_TICKS(30));
  }

  turnOffLeds();
  noTone(BUZZER_PIN);

  // Apply final Light behavior based on menu outcome
  if (lightOn && WiFi.status() == WL_CONNECTED)
  {
    if (userLightChoice == CHOICE_TURN_OFF_NOW)
    {
      sendOffPostRequest(true); // Instant OFF (no transition)
    }
    else if (userLightChoice == CHOICE_KEEP_PREVIOUS)
    {
      if (initialLightWasOn)
      {
        sendOnPostRequest(true); // Restore ON instantly
      }
      else
      {
        sendOffPostRequest(true); // Restore OFF instantly
      }
    }
    else
    {
      // Default Behavior (Timeout, Back button, or Touch): 5-minute delayed background turn off with transition
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
      
      // ArduinoJson v7 structure
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