#include "EEPROM.h"

Preferences preferences;

void saveAlarms() {
    preferences.begin("alarms", false);

    for (int i = 0; i < 7; ++i) {
        preferences.putBool(("isSet" + String(i)).c_str(), alarms[i].isSet);
        preferences.putInt(("hours" + String(i)).c_str(), alarms[i].hours);
        preferences.putInt(("minutes" + String(i)).c_str(), alarms[i].minutes);
        preferences.putBool(("soundOn" + String(i)).c_str(), alarms[i].soundOn); // Adding soundOn
    }

    preferences.end();
}

void readAlarms() {
    preferences.begin("alarms", false);

    for (int i = 0; i < 7; ++i) {
        alarms[i].isSet = preferences.getBool(("isSet" + String(i)).c_str(), false);
        alarms[i].hours = preferences.getInt(("hours" + String(i)).c_str(), 0);
        alarms[i].minutes = preferences.getInt(("minutes" + String(i)).c_str(), 0);
        alarms[i].soundOn = preferences.getBool(("soundOn" + String(i)).c_str(), false); // Reading soundOn
    }

    preferences.end();
}