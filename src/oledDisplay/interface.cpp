#include "interface.h"

using namespace Menu;

bool menuRunning = false;

unsigned long startTime;  // Variable to store the start time
const unsigned long delayTime = 6000;  // 10 minutes in milliseconds

const colorDef<uint16_t> colors[6] MEMMODE={
  {{0,0},{0,1,1}},//bgColor
  {{1,1},{1,0,0}},//fgColor
  {{1,1},{1,0,0}},//valColor
  {{1,1},{1,0,0}},//unitColor
  {{0,1},{0,0,1}},//cursorColor
  {{1,1},{1,0,0}},//titleColor
};

TOGGLE(alarms[1].isSet,mondayToggle,"Monday:",doNothing,noEvent,noStyle//,doExit,enterEvent,noStyle
  ,VALUE("On",true,doNothing,noEvent)
  ,VALUE("Off",false,doNothing,noEvent)
);

TOGGLE(alarms[1].soundOn,mondayToggleRing,"Ring alarm:",doNothing,noEvent,noStyle//,doExit,enterEvent,noStyle
  ,VALUE("True",true,doNothing,noEvent)
  ,VALUE("False",false,doNothing,noEvent)
);

MENU(Monday, "Monday", Menu::doNothing, Menu::noEvent, Menu::wrapStyle
     , SUBMENU(mondayToggle)
      , SUBMENU(mondayToggleRing)
     , FIELD(alarms[1].hours,"Hours:","",0,24,1,0,doNothing,noEvent,noStyle)
     , FIELD(alarms[1].minutes,"Minutes:","",0,60,10,1,doNothing,noEvent,noStyle)
     , EXIT("<Back")
    );

TOGGLE(alarms[2].isSet, tuesdayToggle, "Tuesday:", doNothing, noEvent, noStyle
  , VALUE("On", true, doNothing, noEvent)
  , VALUE("Off", false, doNothing, noEvent)
);

TOGGLE(alarms[2].soundOn,tuesdayToggleRing,"Ring alarm:",doNothing,noEvent,noStyle//,doExit,enterEvent,noStyle
  ,VALUE("True",true,doNothing,noEvent)
  ,VALUE("False",false,doNothing,noEvent)
);

MENU(Tuesday, "Tuesday", Menu::doNothing, Menu::noEvent, Menu::wrapStyle
      , SUBMENU(tuesdayToggle)
      , SUBMENU(tuesdayToggleRing)
      , FIELD(alarms[2].hours, "Hours:", "", 0, 24, 1, 0, doNothing, noEvent, noStyle)
      , FIELD(alarms[2].minutes, "Minutes:", "", 0, 60, 10, 1, doNothing, noEvent, noStyle)
      , EXIT("<Back")
);

TOGGLE(alarms[3].isSet, wednesdayToggle, "Wednesday:", doNothing, noEvent, noStyle
  , VALUE("On", true, doNothing, noEvent)
  , VALUE("Off", false, doNothing, noEvent)
);

TOGGLE(alarms[3].soundOn, wednsedayToggleRing,"Ring alarm:",doNothing,noEvent,noStyle//,doExit,enterEvent,noStyle
  ,VALUE("True",true,doNothing,noEvent)
  ,VALUE("False",false,doNothing,noEvent)
);

MENU(Wednesday, "Wednesday", Menu::doNothing, Menu::noEvent, Menu::wrapStyle
      , SUBMENU(wednesdayToggle)
      , SUBMENU(wednsedayToggleRing)
      , FIELD(alarms[3].hours, "Hours:", "", 0, 24, 1, 0, doNothing, noEvent, noStyle)
      , FIELD(alarms[3].minutes, "Minutes:", "", 0, 60, 10, 1, doNothing, noEvent, noStyle)
      , EXIT("<Back")
);

TOGGLE(alarms[4].isSet, thursdayToggle, "Thursday:", doNothing, noEvent, noStyle
  , VALUE("On", true, doNothing, noEvent)
  , VALUE("Off", false, doNothing, noEvent)
);

TOGGLE(alarms[4].soundOn, thursdayToggleRing,"Ring alarm:",doNothing,noEvent,noStyle//,doExit,enterEvent,noStyle
  ,VALUE("True",true,doNothing,noEvent)
  ,VALUE("False",false,doNothing,noEvent)
);

MENU(Thursday, "Thursday", Menu::doNothing, Menu::noEvent, Menu::wrapStyle
      , SUBMENU(thursdayToggle)
      , SUBMENU(thursdayToggleRing)
      , FIELD(alarms[4].hours, "Hours:", "", 0, 24, 1, 0, doNothing, noEvent, noStyle)
      , FIELD(alarms[4].minutes, "Minutes:", "", 0, 60, 10, 1, doNothing, noEvent, noStyle)
      , EXIT("<Back")
);


TOGGLE(alarms[5].isSet, fridayToggle, "Friday:", doNothing, noEvent, noStyle
  , VALUE("On", true, doNothing, noEvent)
  , VALUE("Off", false, doNothing, noEvent)
);

TOGGLE(alarms[5].soundOn, fridayToggleRing,"Ring alarm:",doNothing,noEvent,noStyle//,doExit,enterEvent,noStyle
  ,VALUE("True",true,doNothing,noEvent)
  ,VALUE("False",false,doNothing,noEvent)
);

MENU(Friday, "Friday", Menu::doNothing, Menu::noEvent, Menu::wrapStyle
      , SUBMENU(fridayToggle)
      , SUBMENU(fridayToggleRing)
      , FIELD(alarms[5].hours, "Hours:", "", 0, 24, 1, 0, doNothing, noEvent, noStyle)
      , FIELD(alarms[5].minutes, "Minutes:", "", 0, 60, 10, 1, doNothing, noEvent, noStyle)
      , EXIT("<Back")
);

TOGGLE(alarms[6].isSet, saturdayToggle, "Saturday:", doNothing, noEvent, noStyle
  , VALUE("On", true, doNothing, noEvent)
  , VALUE("Off", false, doNothing, noEvent)
);

TOGGLE(alarms[6].soundOn, saturdayToggleRing,"Ring alarm:",doNothing,noEvent,noStyle//,doExit,enterEvent,noStyle
  ,VALUE("True",true,doNothing,noEvent)
  ,VALUE("False",false,doNothing,noEvent)
);

MENU(Saturday, "Saturday", Menu::doNothing, Menu::noEvent, Menu::wrapStyle
      , SUBMENU(saturdayToggle)
      , SUBMENU(saturdayToggleRing)
      , FIELD(alarms[6].hours, "Hours:", "", 0, 24, 1, 0, doNothing, noEvent, noStyle)
      , FIELD(alarms[6].minutes, "Minutes:", "", 0, 60, 10, 1, doNothing, noEvent, noStyle)
      , EXIT("<Back")
);

TOGGLE(alarms[0].isSet, sundayToggle, "Sunday:", doNothing, noEvent, noStyle
  , VALUE("On", true, doNothing, noEvent)
  , VALUE("Off", false, doNothing, noEvent)
);

TOGGLE(alarms[0].soundOn, sundayToggleRing,"Ring alarm:",doNothing,noEvent,noStyle//,doExit,enterEvent,noStyle
  ,VALUE("True",true,doNothing,noEvent)
  ,VALUE("False",false,doNothing,noEvent)
);

MENU(Sunday, "Sunday", Menu::doNothing, Menu::noEvent, Menu::wrapStyle
      , SUBMENU(sundayToggle)
      , SUBMENU(sundayToggleRing)
      , FIELD(alarms[0].hours, "Hours:", "", 0, 24, 1, 0, doNothing, noEvent, noStyle)
      , FIELD(alarms[0].minutes, "Minutes:", "", 0, 60, 10, 1, doNothing, noEvent, noStyle)
      , EXIT("<Back")
);

result saveAlarms(eventMask e, navNode &nav, prompt &item)
{
  saveAlarms();
  return proceed;
}

result enableAlarms(eventMask e, navNode &nav, prompt &item)
{
  enableAllAlarms();
  return proceed;
}

result disableAlarms(eventMask e, navNode &nav, prompt &item)
{
  disableAllAlarms();
  return proceed;
}


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
  display.display();
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
  display.display();
  refreshMenu();
      display.clearDisplay();
    display.display();
  return proceed;
      display.clearDisplay();
    display.display();
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
  display.stopscroll();
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
  display.stopscroll();
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
  display.stopscroll();
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
  display.stopscroll();
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
  display.stopscroll();
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
  display.stopscroll();
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
  display.stopscroll();
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
  display.stopscroll();
  display.clearDisplay();
  refreshMenu();
  return proceed;
}

MENU(debug, "Debug Menu", Menu::doExit, Menu::noEvent, Menu::wrapStyle
     , OP("WiFi", showWifiDebugMenu, enterEvent)
     , OP("CPU", showCPUDebugMenu, enterEvent)
     , OP("General", showGeneralDebugMenu, enterEvent)
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


keyMap joystickBtn_map[] = {
  { -BUTTON_CONFIRM_PIN, defaultNavCodes[enterCmd].ch} ,
  { -BUTTON_UP_PIN, defaultNavCodes[upCmd].ch} ,
  { -BUTTON_DOWN_PIN, defaultNavCodes[downCmd].ch}  ,
  { -BUTTON_EXIT_PIN, defaultNavCodes[escCmd].ch}  ,
};
keyIn<TOTAL_NAV_BUTTONS> joystickBtns(joystickBtn_map);

MENU_OUTPUTS(out,MAX_DEPTH
  ,ADAGFX_OUT(display,colors,fontX,fontY,{0,0,SCREEN_WIDTH/fontX,SCREEN_HEIGHT/fontY})
  ,SERIAL_OUT(Serial)
);

NAVROOT(nav, mainMenu, MAX_DEPTH, joystickBtns, out);


const panel panels[] MEMMODE = {{0, 0, 128 / fontX, 64 / fontY}};
navNode* nodes[sizeof(panels) / sizeof(panel)]; //navNodes to store navigation status
panelsList pList(panels, nodes, 1); //a list of panels and nodes
idx_t tops[MAX_DEPTH] = {0, 0}; //store cursor positions for each level

void initMenus() {
  nav.timeOut = 30;
  joystickBtns.begin();
}

void handleMenus() {
  nav.poll();

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
  display.display();
  refreshMenu();
  nav.idleOff();
  refreshMenu();
  display.clearDisplay();
  display.display();
}