#include "interface.h"

using namespace Menu;

bool menuRunning = false;

unsigned long startTime;  // Variable to store the start time
const unsigned long delayTime = 600000;  // 10 minutes in milliseconds

const colorDef<uint16_t> colors[6] MEMMODE={
  {{0,0},{0,1,1}},//bgColor
  {{1,1},{1,0,0}},//fgColor
  {{1,1},{1,0,0}},//valColor
  {{1,1},{1,0,0}},//unitColor
  {{0,1},{0,0,1}},//cursorColor
  {{1,1},{1,0,0}},//titleColor
};

TOGGLE(days[1].isSet,mondayToggle,"Monday:",doNothing,noEvent,noStyle//,doExit,enterEvent,noStyle
  ,VALUE("On",true,doNothing,noEvent)
  ,VALUE("Off",false,doNothing,noEvent)
);

MENU(Monday, "Monday", Menu::doNothing, Menu::noEvent, Menu::wrapStyle
     , SUBMENU(mondayToggle)
     , FIELD(days[1].hours,"Hours:","",0,24,1,0,doNothing,noEvent,noStyle)
     , FIELD(days[1].minutes,"Minutes:","",0,60,10,1,doNothing,noEvent,noStyle)
     , EXIT("<Back")
    );

TOGGLE(days[2].isSet, tuesdayToggle, "Tuesday:", doNothing, noEvent, noStyle
  , VALUE("On", true, doNothing, noEvent)
  , VALUE("Off", false, doNothing, noEvent)
);

MENU(Tuesday, "Tuesday", Menu::doNothing, Menu::noEvent, Menu::wrapStyle
      , SUBMENU(tuesdayToggle)
      , FIELD(days[2].hours, "Hours:", "", 0, 24, 1, 0, doNothing, noEvent, noStyle)
      , FIELD(days[2].minutes, "Minutes:", "", 0, 60, 10, 1, doNothing, noEvent, noStyle)
      , EXIT("<Back")
);

TOGGLE(days[3].isSet, wednesdayToggle, "Wednesday:", doNothing, noEvent, noStyle
  , VALUE("On", true, doNothing, noEvent)
  , VALUE("Off", false, doNothing, noEvent)
);

MENU(Wednesday, "Wednesday", Menu::doNothing, Menu::noEvent, Menu::wrapStyle
      , SUBMENU(wednesdayToggle)
      , FIELD(days[3].hours, "Hours:", "", 0, 24, 1, 0, doNothing, noEvent, noStyle)
      , FIELD(days[3].minutes, "Minutes:", "", 0, 60, 10, 1, doNothing, noEvent, noStyle)
      , EXIT("<Back")
);

TOGGLE(days[4].isSet, thursdayToggle, "Thursday:", doNothing, noEvent, noStyle
  , VALUE("On", true, doNothing, noEvent)
  , VALUE("Off", false, doNothing, noEvent)
);

MENU(Thursday, "Thursday", Menu::doNothing, Menu::noEvent, Menu::wrapStyle
      , SUBMENU(thursdayToggle)
      , FIELD(days[4].hours, "Hours:", "", 0, 24, 1, 0, doNothing, noEvent, noStyle)
      , FIELD(days[4].minutes, "Minutes:", "", 0, 60, 10, 1, doNothing, noEvent, noStyle)
      , EXIT("<Back")
);

TOGGLE(days[5].isSet, fridayToggle, "Friday:", doNothing, noEvent, noStyle
  , VALUE("On", true, doNothing, noEvent)
  , VALUE("Off", false, doNothing, noEvent)
);

MENU(Friday, "Friday", Menu::doNothing, Menu::noEvent, Menu::wrapStyle
      , SUBMENU(fridayToggle)
      , FIELD(days[5].hours, "Hours:", "", 0, 24, 1, 0, doNothing, noEvent, noStyle)
      , FIELD(days[5].minutes, "Minutes:", "", 0, 60, 10, 1, doNothing, noEvent, noStyle)
      , EXIT("<Back")
);

TOGGLE(days[6].isSet, saturdayToggle, "Saturday:", doNothing, noEvent, noStyle
  , VALUE("On", true, doNothing, noEvent)
  , VALUE("Off", false, doNothing, noEvent)
);

MENU(Saturday, "Saturday", Menu::doNothing, Menu::noEvent, Menu::wrapStyle
      , SUBMENU(saturdayToggle)
      , FIELD(days[6].hours, "Hours:", "", 0, 24, 1, 0, doNothing, noEvent, noStyle)
      , FIELD(days[6].minutes, "Minutes:", "", 0, 60, 10, 1, doNothing, noEvent, noStyle)
      , EXIT("<Back")
);

TOGGLE(days[0].isSet, sundayToggle, "Sunday:", doNothing, noEvent, noStyle
  , VALUE("On", true, doNothing, noEvent)
  , VALUE("Off", false, doNothing, noEvent)
);

MENU(Sunday, "Sunday", Menu::doNothing, Menu::noEvent, Menu::wrapStyle
      , SUBMENU(sundayToggle)
      , FIELD(days[0].hours, "Hours:", "", 0, 24, 1, 0, doNothing, noEvent, noStyle)
      , FIELD(days[0].minutes, "Minutes:", "", 0, 60, 10, 1, doNothing, noEvent, noStyle)
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
  initTempGraph();
  sleepMenu();
  menuRunning = true;
  while (digitalRead(BUTTON_EXIT_PIN))
  { 
    loopTempGraph();
    unsigned long currentTime = millis();  // Get the current time
    if (currentTime - startTime >= delayTime) {
    break;
    startTime = currentTime;
    }
  }
  
  display.clearDisplay();
  refreshMenu();
  return proceed;
}

result showLightChart()
{
  initLightGraph();
  sleepMenu();
  menuRunning = true;
  while (digitalRead(BUTTON_EXIT_PIN))
  { 
    loopLightGraph();
    unsigned long currentTime = millis();  // Get the current time
    if (currentTime - startTime >= delayTime) {
    break;
    startTime = currentTime;
    }
  }
  
  display.clearDisplay();
  refreshMenu();
  return proceed;
}

result showCurrentWeather()
{
  menuRunning = true;
  sleepMenu();
  todaysWeather();
  delay(100);
  do {
    todaysWeather();
    unsigned long currentTime = millis();  // Get the current time
    if (currentTime - startTime >= delayTime) {
    break;
    startTime = currentTime;
    }
    while(digitalRead(BUTTON_EXIT_PIN));
  } while(digitalRead(BUTTON_EXIT_PIN));
  display.stopscroll();
  display.clearDisplay();
  refreshMenu();
  return proceed;
}

result showTomorrowsWeather()
{
  menuRunning = true;
  sleepMenu();
  tommorowsWeather();
  delay(100);
  do {
    tommorowsWeather();
    unsigned long currentTime = millis();  // Get the current time
    if (currentTime - startTime >= delayTime) {
    break;
    startTime = currentTime;
    }
    while(digitalRead(BUTTON_EXIT_PIN));
  } while(digitalRead(BUTTON_EXIT_PIN));
  display.stopscroll();
  display.clearDisplay();
  refreshMenu();
  return proceed;
}

result showDaysAfterWeather()
{
  menuRunning = true;
  sleepMenu();
  daysAfterWeather();
  delay(100);
  do {
    daysAfterWeather();
    unsigned long currentTime = millis();  // Get the current time
    if (currentTime - startTime >= delayTime) {
    break;
    startTime = currentTime;
    }
    while(digitalRead(BUTTON_EXIT_PIN));
  } while(digitalRead(BUTTON_EXIT_PIN));
  display.stopscroll();
  display.clearDisplay();
  refreshMenu();
  return proceed;
}

MENU(sensors, "Sensor Menu", Menu::doExit, Menu::noEvent, Menu::wrapStyle
     , OP("Temperature", showTempChart, enterEvent)
     , OP("Light", showLightChart, enterEvent)
     , EXIT("<Back")
    );

MENU(weatherMenu, "Weather Menu", Menu::doExit, Menu::noEvent, Menu::wrapStyle
     , OP("Current Weather", showCurrentWeather, enterEvent)
     , OP("Tomorrow's cast", showTomorrowsWeather, enterEvent)
     , OP("Day After's Cast", showDaysAfterWeather, enterEvent)
     , EXIT("<Back")
    );

MENU(mainMenu, "Main Menu", Menu::doNothing, Menu::noEvent, Menu::wrapStyle
  , SUBMENU(alarmMenu)
  , SUBMENU(weatherMenu)
  , SUBMENU(sensors)
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