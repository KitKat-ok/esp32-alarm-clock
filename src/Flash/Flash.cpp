#include "Flash.h"

Preferences preferences;

void saveAlarms()
{
    preferences.begin("alarms", false);

    for (int i = 0; i < MAX_ALARMS; ++i)
    {
        preferences.putBool(("exists" + String(i)).c_str(), alarms[i].exists);
        preferences.putBool(("enabled" + String(i)).c_str(), alarms[i].enabled);
        preferences.putInt(("hours" + String(i)).c_str(), alarms[i].hours);
        preferences.putInt(("minutes" + String(i)).c_str(), alarms[i].minutes);
        preferences.putBool(("soundOn" + String(i)).c_str(), alarms[i].soundOn); // Adding soundOn
    }
    preferences.end();
}

void readAlarms()
{
    preferences.begin("alarms", false);

    for (int i = 0; i < MAX_ALARMS; ++i)
    {
        alarms[i].exists = preferences.getBool(("exists" + String(i)).c_str(), false);
        alarms[i].enabled = preferences.getBool(("enabled" + String(i)).c_str(), false);
        alarms[i].hours = preferences.getInt(("hours" + String(i)).c_str(), 0);
        alarms[i].minutes = preferences.getInt(("minutes" + String(i)).c_str(), 0);
        alarms[i].soundOn = preferences.getBool(("soundOn" + String(i)).c_str(), false); // Reading soundOn
    }

    preferences.end();
}

void saveOtaValue(bool Value)
{
    preferences.begin("Ota", false);

    preferences.putBool("Ota Enabled", Value);
    preferences.end();
}

void readOtaValue()
{
    preferences.begin("Ota", false);

    OTAEnabled = preferences.getBool("Ota Enabled", false);
    preferences.end();
}