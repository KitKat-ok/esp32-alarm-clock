#include "interface.h"

using namespace Menu;

void showEvent();


const colorDef<uint16_t> colors[6] MEMMODE={
  {{0,0},{0,1,1}},//bgColor
  {{1,1},{1,0,0}},//fgColor
  {{1,1},{1,0,0}},//valColor
  {{1,1},{1,0,0}},//unitColor
  {{0,1},{0,0,1}},//cursorColor
  {{1,1},{1,0,0}},//titleColor
};

#define LEDPIN LED_BUILTIN

int ledCtrl=HIGH;

TOGGLE(monday,mondayToggle,"Monday:",doNothing,noEvent,noStyle//,doExit,enterEvent,noStyle
  ,VALUE("On",true,doNothing,noEvent)
  ,VALUE("Off",false,doNothing,noEvent)
);

MENU(Monday, "Monday", Menu::doNothing, Menu::noEvent, Menu::wrapStyle
      ,SUBMENU(mondayToggle)
     ,FIELD(hoursMonday,"Hours:","",0,24,1,0,doNothing,noEvent,noStyle)
      ,FIELD(minutesMonday,"Minutes:","",0,60,10,1,doNothing,noEvent,noStyle)
     , EXIT("<Back")
    );

    TOGGLE(tuesday, tuesdayToggle, "Tuesday:", doNothing, noEvent, noStyle
  , VALUE("On", true, doNothing, noEvent)
  , VALUE("Off", false, doNothing, noEvent)
);

MENU(Tuesday, "Tuesday", Menu::doNothing, Menu::noEvent, Menu::wrapStyle
      , SUBMENU(tuesdayToggle)
      , FIELD(hoursTuesday, "Hours:", "", 0, 24, 1, 0, doNothing, noEvent, noStyle)
      , FIELD(minutesTuesday, "Minutes:", "", 0, 60, 10, 1, doNothing, noEvent, noStyle)
      , EXIT("<Back")
);

TOGGLE(wednesday, wednesdayToggle, "Wednesday:", doNothing, noEvent, noStyle
  , VALUE("On", true, doNothing, noEvent)
  , VALUE("Off", false, doNothing, noEvent)
);

MENU(Wednesday, "Wednesday", Menu::doNothing, Menu::noEvent, Menu::wrapStyle
      , SUBMENU(wednesdayToggle)
      , FIELD(hoursWednesday, "Hours:", "", 0, 24, 1, 0, doNothing, noEvent, noStyle)
      , FIELD(minutesWednesday, "Minutes:", "", 0, 60, 10, 1, doNothing, noEvent, noStyle)
      , EXIT("<Back")
);

TOGGLE(thursday, thursdayToggle, "Thursday:", doNothing, noEvent, noStyle
  , VALUE("On", true, doNothing, noEvent)
  , VALUE("Off", false, doNothing, noEvent)
);

MENU(Thursday, "Thursday", Menu::doNothing, Menu::noEvent, Menu::wrapStyle
      , SUBMENU(thursdayToggle)
      , FIELD(hoursThursday, "Hours:", "", 0, 24, 1, 0, doNothing, noEvent, noStyle)
      , FIELD(minutesThursday, "Minutes:", "", 0, 60, 10, 1, doNothing, noEvent, noStyle)
      , EXIT("<Back")
);

TOGGLE(friday, fridayToggle, "Friday:", doNothing, noEvent, noStyle
  , VALUE("On", true, doNothing, noEvent)
  , VALUE("Off", false, doNothing, noEvent)
);

MENU(Friday, "Friday", Menu::doNothing, Menu::noEvent, Menu::wrapStyle
      , SUBMENU(fridayToggle)
      , FIELD(hoursFriday, "Hours:", "", 0, 24, 1, 0, doNothing, noEvent, noStyle)
      , FIELD(minutesFriday, "Minutes:", "", 0, 60, 10, 1, doNothing, noEvent, noStyle)
      , EXIT("<Back")
);

TOGGLE(saturday, saturdayToggle, "Saturday:", doNothing, noEvent, noStyle
  , VALUE("On", true, doNothing, noEvent)
  , VALUE("Off", false, doNothing, noEvent)
);

MENU(Saturday, "Saturday", Menu::doNothing, Menu::noEvent, Menu::wrapStyle
      , SUBMENU(saturdayToggle)
      , FIELD(hoursSaturday, "Hours:", "", 0, 24, 1, 0, doNothing, noEvent, noStyle)
      , FIELD(minutesSaturday, "Minutes:", "", 0, 60, 10, 1, doNothing, noEvent, noStyle)
      , EXIT("<Back")
);

TOGGLE(sunday, sundayToggle, "Sunday:", doNothing, noEvent, noStyle
  , VALUE("On", true, doNothing, noEvent)
  , VALUE("Off", false, doNothing, noEvent)
);

MENU(Sunday, "Sunday", Menu::doNothing, Menu::noEvent, Menu::wrapStyle
      , SUBMENU(sundayToggle)
      , FIELD(hoursSunday, "Hours:", "", 0, 24, 1, 0, doNothing, noEvent, noStyle)
      , FIELD(minutesSunday, "Minutes:", "", 0, 60, 10, 1, doNothing, noEvent, noStyle)
      , EXIT("<Back")
);

MENU(alarmMenu, "Alarm Menu", Menu::doNothing, Menu::noEvent, Menu::wrapStyle
      , SUBMENU(Monday)
      , SUBMENU(Tuesday)
      , SUBMENU(Wednesday)
      , SUBMENU(Thursday)
      , SUBMENU(Friday)
      , SUBMENU(Saturday)
      , SUBMENU(Sunday)
      , EXIT("<Back")
);

MENU(weatherMenu, "Weather Menu", Menu::doNothing, Menu::noEvent, Menu::wrapStyle
     , OP("Sub1", showEvent, enterEvent)
     , EXIT("<Back")
    );

    result showEvent(eventMask e, navNode &nav, prompt &item)
{
  do {
    showMainPage();
  } while(digitalRead(BUTTON_EXIT_PIN));
  return proceed;
}

MENU(mainMenu, "Main Menu", Menu::doNothing, Menu::noEvent, Menu::wrapStyle
  , SUBMENU(alarmMenu)
  , SUBMENU(weatherMenu)
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


void showEvent() {
  Serial.println("test");
}