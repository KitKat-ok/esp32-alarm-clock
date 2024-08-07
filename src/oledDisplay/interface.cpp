#include "interface.h"

using namespace Menu;

bool menuRunning = false;

unsigned long startTime;  // Variable to store the start time
const unsigned long delayTime = 6000;  // 10 minutes in milliseconds

// Define color scheme
const colorDef<uint16_t> colors[6] MEMMODE = {
  {{0,0},{0,1,1}},  // bgColor
  {{1,1},{1,0,0}},  // fgColor
  {{1,1},{1,0,0}},  // valColor
  {{1,1},{1,0,0}},  // unitColor
  {{0,1},{0,0,1}},  // cursorColor
  {{1,1},{1,0,0}}   // titleColor
};

// Helper Functions
result saveAlarms(eventMask e, navNode &nav, prompt &item) {
  saveAlarms();
  return proceed;
}

result enableAlarms(eventMask e, navNode &nav, prompt &item) {
  enableAllAlarms();
  return proceed;
}

result disableAlarms(eventMask e, navNode &nav, prompt &item) {
  disableAllAlarms();
  return proceed;
}

result startOTA() {
  saveOtaValue(true);
  ESP.restart();
  return proceed;
}

result softwareReset() {
  ESP.restart();
  return proceed;
}

result showTempChart()
{
  sleepMenu();
  initTempGraph();
  startTime = millis(); 
  menuRunning = true;
  while (digitalRead(BUTTON_EXIT_PIN))
  { 
    loopTempGraph();
    if (millis() - startTime >= 15 * 60 * 1000) {
      break;
    }
  }
  
  display.clearDisplay();
  manager.oledDisplay();
  refreshMenu();
  return proceed;
}

result showLightChart()
{
  sleepMenu();
  initLightGraph();
  startTime = millis(); 
  menuRunning = true;
  while (digitalRead(BUTTON_EXIT_PIN))
  { 
    loopLightGraph();
    if (millis() - startTime >= 15 * 60 * 1000) {
      break;
    }
  }
  
  display.clearDisplay();
  manager.oledDisplay();
  refreshMenu();
      display.clearDisplay();
    manager.oledDisplay();
  return proceed;
      display.clearDisplay();
    manager.oledDisplay();
}

result showCurrentWeather()
{
  menuRunning = true;
  startTime = millis(); 
  sleepMenu();
  currentWeather();
  delay(100);
  while(digitalRead(BUTTON_EXIT_PIN)) {
        if (millis() - startTime >= 1 * 60 * 1000) {
      break;
  }}
  manager.stopScrolling();
  display.clearDisplay();
  refreshMenu();
  return proceed;
}

result showTodaysWeather()
{
  menuRunning = true;
  sleepMenu();
  displayWeatherCast(0);
  startTime = millis(); 
  delay(100);
  while(digitalRead(BUTTON_EXIT_PIN)) {
  if (millis() - startTime >= 15 * 60 * 1000) {
      break;
  }
  }
  manager.stopScrolling();
  display.clearDisplay();
  refreshMenu();
  return proceed;
}

result showTomorrowsWeather()
{
  menuRunning = true;
  sleepMenu();
  displayWeatherCast(1);
  startTime = millis(); 
  delay(100);
  while(digitalRead(BUTTON_EXIT_PIN)) {
  if (millis() - startTime >= 15 * 60 * 1000) {
      break;
  }
  }
  manager.stopScrolling();
  display.clearDisplay();
  refreshMenu();
  return proceed;
}

result showDaysAfterWeather()
{
  menuRunning = true;
  sleepMenu();
  displayWeatherCast(2);
  startTime = millis(); 
  delay(100);
  while(digitalRead(BUTTON_EXIT_PIN)) {
  if (millis() - startTime >= 15 * 60 * 1000) {
      break;
  }
  }
  manager.stopScrolling();
  display.clearDisplay();
  refreshMenu();
  return proceed;
}
result showWifiDebugMenu()
{
  menuRunning = true;
  sleepMenu();
  wifiDebugMenu();
  startTime = millis(); 
  delay(100);
  while(digitalRead(BUTTON_EXIT_PIN)) {
  if (millis() - startTime >= 15 * 60 * 1000) {
      break;
  }
    wifiDebugMenu();
  }
  manager.stopScrolling();
  display.clearDisplay();
  refreshMenu();
  return proceed;
}
result showCPUDebugMenu()
{
  menuRunning = true;
  sleepMenu();
  CPUDebugMenu();
  startTime = millis(); 
  delay(100);
  while(digitalRead(BUTTON_EXIT_PIN)) {
  if (millis() - startTime >= 15 * 60 * 1000) {
      break;
  }
    CPUDebugMenu();
  }
  manager.stopScrolling();
  display.clearDisplay();
  refreshMenu();
  return proceed;
}
result showGeneralDebugMenu()
{
  menuRunning = true;
  sleepMenu();
  generalDebugMenu();
  startTime = millis(); 
  delay(100);
  while(digitalRead(BUTTON_EXIT_PIN)) {
  if (millis() - startTime >= 15 * 60 * 1000) {
      break;
  }
    generalDebugMenu();
  }
  manager.stopScrolling();
  display.clearDisplay();
  refreshMenu();
  return proceed;
}

result showFpsMenu()
{
  menuRunning = true;
  sleepMenu();
  fpsCalc();
  startTime = millis(); 
  delay(100);
  while(digitalRead(BUTTON_EXIT_PIN)) {
  if (millis() - startTime >= 15 * 60 * 1000) {
      break;
  }
    fpsCalc();
  }
  manager.stopScrolling();
  display.clearDisplay();
  refreshMenu();
  return proceed;
}

result showCallendarMenu()
{
  time_t currentTime = now();              // Get the current time
  int weekdayIndex = weekday(currentTime); // Get the day of the week (1 = Sunday, 2 = Monday, etc.)
  menuRunning = true;
  sleepMenu();
  showCalendar(weekdayIndex,2024);
  startTime = millis(); 
  delay(100);
  while(digitalRead(BUTTON_EXIT_PIN)) {
  if (millis() - startTime >= 15 * 60 * 1000) {
      break;
  }
    loopCallendar();
  }
  manager.stopScrolling();
  display.clearDisplay();
  refreshMenu();
  return proceed;
}

result syncTime() {
  synchronizeAndSetTime();
  return proceed;
}

result fixDisplay() {
manager.sendCustomCommand(0xD3);
manager.sendCustomCommand(0);
 return proceed;
}


// Alarm Toggles
#define DEFINE_ALARM_TOGGLE(day, index) \
  TOGGLE(alarms[index].isSet, day##Toggle, #day ":", doNothing, noEvent, noStyle, \
    VALUE("On", true, doNothing, noEvent), \
    VALUE("Off", false, doNothing, noEvent) \
  ); \
  TOGGLE(alarms[index].soundOn, day##ToggleRing, "Ring alarm:", doNothing, noEvent, noStyle, \
    VALUE("True", true, doNothing, noEvent), \
    VALUE("False", false, doNothing, noEvent) \
  ); \
  MENU(day, #day, Menu::doNothing, Menu::noEvent, Menu::wrapStyle, \
    SUBMENU(day##Toggle), \
    SUBMENU(day##ToggleRing), \
    FIELD(alarms[index].hours, "Hours:", "", 0, 24, 1, 0, doNothing, noEvent, noStyle), \
    FIELD(alarms[index].minutes, "Minutes:", "", 0, 60, 10, 1, doNothing, noEvent, noStyle), \
    EXIT("<Back") \
  )

DEFINE_ALARM_TOGGLE(Monday, 1);
DEFINE_ALARM_TOGGLE(Tuesday, 2);
DEFINE_ALARM_TOGGLE(Wednesday, 3);
DEFINE_ALARM_TOGGLE(Thursday, 4);
DEFINE_ALARM_TOGGLE(Friday, 5);
DEFINE_ALARM_TOGGLE(Saturday, 6);
DEFINE_ALARM_TOGGLE(Sunday, 0);

MENU(alarmMenu, "Alarm Menu", Menu::doNothing, Menu::noEvent, Menu::wrapStyle
      , SUBMENU(Monday)
      , SUBMENU(Tuesday)
      , SUBMENU(Wednesday)
      , SUBMENU(Thursday)
      , SUBMENU(Friday)
      , SUBMENU(Saturday)
      , SUBMENU(Sunday)
      , OP("Save alarms ---------------", saveAlarms, enterEvent)
      , OP("Disable alarms", disableAlarms, enterEvent)
      , OP("Enable alarms", enableAlarms, enterEvent)
      , EXIT("<Back")
);

MENU(debug, "Debug Menu", Menu::doExit, Menu::noEvent, Menu::wrapStyle
     , OP("Fix Display...", fixDisplay, enterEvent)
     , OP("WiFi", showWifiDebugMenu, enterEvent)
     , OP("CPU", showCPUDebugMenu, enterEvent)
     , OP("General", showGeneralDebugMenu, enterEvent)
     , OP("Fps calc", showFpsMenu, enterEvent)
     , OP("Start OTA", startOTA, enterEvent)
     , OP("Software Reset", softwareReset, enterEvent)
     , OP("Synchronize Time", syncTime, enterEvent)
     , EXIT("<Back")
    );

MENU(sensors, "Sensors Menu", Menu::doExit, Menu::noEvent, Menu::wrapStyle
     , OP("Temperature", showTempChart, enterEvent)
     , OP("Light", showLightChart, enterEvent)
     , EXIT("<Back")
    );

MENU(info, "Info Menu", Menu::doExit, Menu::noEvent, Menu::wrapStyle
     , OP("Callendar", showCallendarMenu, enterEvent)
     , SUBMENU(sensors)
     , SUBMENU(debug)
     , EXIT("<Back")
    );

MENU(menus, "Weather Menu", Menu::doExit, Menu::noEvent, Menu::wrapStyle
     , OP("Current Weather", showCurrentWeather, enterEvent)
     , OP("Today's cast", showTodaysWeather, enterEvent)
     , OP("Tomorrow's cast", showTomorrowsWeather, enterEvent)
     , OP("Day After's Cast", showDaysAfterWeather, enterEvent)
     , EXIT("<Back")
    );

MENU(mainMenu, "Main Menu", Menu::doNothing, Menu::noEvent, Menu::wrapStyle
  , SUBMENU(alarmMenu)
  , SUBMENU(menus)
  , SUBMENU(info)
  ,EXIT("<Back")
);

// Key Mapping and Outputs
keyMap joystickBtn_map[] = {
  { -BUTTON_CONFIRM_PIN, defaultNavCodes[enterCmd].ch },
  { -BUTTON_UP_PIN, defaultNavCodes[upCmd].ch },
  { -BUTTON_DOWN_PIN, defaultNavCodes[downCmd].ch },
  { -BUTTON_EXIT_PIN, defaultNavCodes[escCmd].ch }
};
keyIn<TOTAL_NAV_BUTTONS> joystickBtns(joystickBtn_map);

MENU_OUTPUTS(out, MAX_DEPTH,
  ADAGFX_OUT(display, colors, fontX, fontY, {0, 0, SCREEN_WIDTH / fontX, SCREEN_HEIGHT / fontY}),
  SERIAL_OUT(Serial)
);

NAVROOT(nav, mainMenu, MAX_DEPTH, joystickBtns, out);

const panel panels[] MEMMODE = { {0, 0, 128 / fontX, 64 / fontY} };
navNode* nodes[sizeof(panels) / sizeof(panel)];  // navNodes to store navigation status
panelsList pList(panels, nodes, 1);  // a list of panels and nodes
idx_t tops[sizeof(panels) / sizeof(panel)];


void initMenus() {
  nav.timeOut = 30;
  joystickBtns.begin();
}

bool sleeping = false;

void handleMenus() {
  nav.poll();

  if (nav.idleChanged)
  {
    display.clearDisplay();
    manager.oledDisplay();
  }
  

  if (!nav.sleepTask) {
    manager.oledDisplay();    
    manager.stopScrolling();
  }
  

  if (nav.sleepTask) {
    showMainPage();
  }
}

void sleepMenu() {
    nav.idleOn();
}

void refreshMenu() {
  nav.refresh();
}

void wakeUpMenu() {
  display.clearDisplay();
  manager.oledDisplay();
  refreshMenu();
  nav.idleOff();
  refreshMenu();
  display.clearDisplay();
  manager.oledDisplay();
}