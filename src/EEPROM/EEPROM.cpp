#include "EEPROM.h"

Preferences preferences;

void saveAlarms() {
    preferences.begin("alarms", false);

    for (int i = 0; i < 7; ++i) {
        preferences.putBool(("isSet" + String(i)).c_str(), days[i].isSet);
        preferences.putInt(("hours" + String(i)).c_str(), days[i].hours);
        preferences.putInt(("minutes" + String(i)).c_str(), days[i].minutes);
    }

    preferences.end();
}

void readAlarms() {
    preferences.begin("alarms", false);

    for (int i = 0; i < 7; ++i) {
        days[i].isSet = preferences.getBool(("isSet" + String(i)).c_str(), false);
        days[i].hours = preferences.getInt(("hours" + String(i)).c_str(), 0);
        days[i].minutes = preferences.getInt(("minutes" + String(i)).c_str(), 0);
    }

    preferences.end();
}
