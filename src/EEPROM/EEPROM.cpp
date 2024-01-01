#include "EEPROM.h"

Preferences preferences;

void saveAlarms() {
    preferences.begin("alarms", false); // Open Preferences with namespace "myApp" and auto-save disabled

    // Save bool variables
    preferences.putBool("sunday", sunday);
    preferences.putBool("monday", monday);
    preferences.putBool("tuesday", tuesday);
    preferences.putBool("wednesday", wednesday);
    preferences.putBool("thursday", thursday);
    preferences.putBool("friday", friday);
    preferences.putBool("saturday", saturday);

    // Save int variables
    preferences.putInt("hoursSunday", hoursSunday);
    preferences.putInt("minutesSunday", minutesSunday);
    preferences.putInt("hoursMonday", hoursMonday);
    preferences.putInt("minutesMonday", minutesMonday);
    preferences.putInt("hoursTuesday", hoursTuesday);
    preferences.putInt("minutesTuesday", minutesTuesday);
    preferences.putInt("hoursWednesday", hoursWednesday);
    preferences.putInt("minutesWednesday", minutesWednesday);
    preferences.putInt("hoursThursday", hoursThursday);
    preferences.putInt("minutesThursday", minutesThursday);
    preferences.putInt("hoursFriday", hoursFriday);
    preferences.putInt("minutesFriday", minutesFriday);
    preferences.putInt("hoursSaturday", hoursSaturday);
    preferences.putInt("minutesSaturday", minutesSaturday);

    preferences.end(); // Close the Preferences
}


void readAlarms() {
    preferences.begin("alarms", false); // Open Preferences with namespace "myApp" and auto-save enabled

    // Read bool variables
    sunday = preferences.getBool("sunday", false);
    monday = preferences.getBool("monday", false);
    tuesday = preferences.getBool("tuesday", false);
    wednesday = preferences.getBool("wednesday", false);
    thursday = preferences.getBool("thursday", false);
    friday = preferences.getBool("friday", false);
    saturday = preferences.getBool("saturday", false);

    // Read int variables
    hoursSunday = preferences.getInt("hoursSunday", 0);
    minutesSunday = preferences.getInt("minutesSunday", 0);
    hoursMonday = preferences.getInt("hoursMonday", 0);
    minutesMonday = preferences.getInt("minutesMonday", 0);
    hoursTuesday = preferences.getInt("hoursTuesday", 0);
    minutesTuesday = preferences.getInt("minutesTuesday", 0);
    hoursWednesday = preferences.getInt("hoursWednesday", 0);
    minutesWednesday = preferences.getInt("minutesWednesday", 0);
    hoursThursday = preferences.getInt("hoursThursday", 0);
    minutesThursday = preferences.getInt("minutesThursday", 0);
    hoursFriday = preferences.getInt("hoursFriday", 0);
    minutesFriday = preferences.getInt("minutesFriday", 0);
    hoursSaturday = preferences.getInt("hoursSaturday", 0);
    minutesSaturday = preferences.getInt("minutesSaturday", 0);

    preferences.end(); // Close the Preferences
}