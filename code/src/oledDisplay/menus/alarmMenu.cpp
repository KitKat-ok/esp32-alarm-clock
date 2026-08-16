#include "alarmMenu.h"

bool AlarmMenuUpdate = true;
uint8_t alarmCurrentState = 0;
bool isEditingAlarm = false;
bool inDaySelectionMode = false;

void deleteAlarm(int index)
{
    alarms[index].exists = false;
    showMenu();
}

void deleteAlarmStatic(int index);
void changeAlarmDay();

Submenu *createAlarmsMenu()
{
    Submenu *alarmsSubmenu = nullptr;
    if (!alarmsSubmenu)
    {
        alarmsSubmenu = createSubmenu("Alarms", MAX_ALARMS + 1); // +1 for the add alarm mhm

        // Add "Add New Alarm" option
        addEntryToSubmenu(alarmsSubmenu, "Add New Alarm", addNewAlarm, nullptr, &DejaVu_LGC_Sans_Bold_10);
    }
    return alarmsSubmenu;
}

Submenu *alarmsSubmenu = createAlarmsMenu();

String getAlarmEntryName(int index)
{
    String days;
    for (int i = 0; i < 7; i++)
    {
        if (alarms[index].days[i])
        {
            if (!days.isEmpty())
                days += ".";
            days += getShortestWeekdayName(i + 1);
        }
    }

    String name = String(index) + " " + String(alarms[index].enabled ? "On" : "Off") + " " + formatWithLeadingZero(alarms[index].hours) + ":" + formatWithLeadingZero(alarms[index].minutes) + " " + days;
    return name;
}

void deleteAlarmStatic(int index)
{
    Serial.println("delete: " + String(index));
    alarms[index].exists = false;
    removeMenuEntry(data.currentButton);
    alarmsSubmenu->count = alarmsSubmenu->count - 1;
    data.currentButton = 0;
    exitLoopFunction = true;
}

void addNewAlarm()
{
    for (int i = 0; i < MAX_ALARMS; ++i)
    {
        if (!alarms[i].exists)
        {
            alarms[i] = {
                // lets not have random data there
                true,                                       // exists
                false,                                      // enabled
                {true, true, true, true, true, true, true}, // days
                0,                                          // hours
                0,                                          // minutes
                true,                                       // soundOn
                true                                        // lightOn
            };
            Serial.println("New alarm added. " + String(i));
            data.currentSubmenu = alarmsSubmenu->entries;
            data.submenuCount = alarmsSubmenu->count;

            addEntryToSubmenu(alarmsSubmenu, getAlarmEntryName(i), initManageAlarm, manageAlarms, &font4pt7b);

            data.submenuCount++;
            data.currentButton = data.submenuCount - 1;
            delay(1);
            showMenu();
            return;
        }
    }
    Serial.println("Failed to add alarm: Maximum number of alarms reached.");
    int rectWidth = 120;
    int rectHeight = 40;
    int rectX = (SCREEN_WIDTH - rectWidth) / 2;
    int rectY = (SCREEN_HEIGHT - rectHeight) / 2;

    oled.fillRect(rectX, rectY, rectWidth, rectHeight, SSD1327_BLACK);
    oled.drawRect(rectX - 1, rectY - 1, rectWidth + 2, rectHeight + 2, SSD1327_WHITE);

    oled.setTextColor(SSD1327_WHITE);
    oled.setCursor(rectX + 10, rectY + 10);
    oled.print("Max alarm");
    oled.setCursor(rectX + 10, rectY + 20);
    oled.print("number reached");
    oledMana.display();
    delay(5000);
}

void initAlarmMenus()
{
    for (int i = 0; i < MAX_ALARMS; ++i)
    {
        Serial.println("Alarm exists: " + String(alarms[i].exists));

        if (alarms[i].exists == true)
        {
            Serial.println("New alarm added. " + String(i));

            addEntryToSubmenu(alarmsSubmenu, getAlarmEntryName(i), initManageAlarm, manageAlarms, &font4pt7b);

            delay(1);
            showMenu();
        }
    }
}

void refreshAlarmsSubmenu()
{
    if (alarmsSubmenu)
    {
        memset(alarmsSubmenu->entries, 0, sizeof(entryMenu) * alarmsSubmenu->maxMenus);
        alarmsSubmenu->count = 0;

        addEntryToSubmenu(alarmsSubmenu, "Add New Alarm", addNewAlarm, nullptr, &DejaVu_LGC_Sans_Bold_10);

        for (int i = 0; i < MAX_ALARMS; ++i)
        {
            if (alarms[i].exists)
            {
                addEntryToSubmenu(alarmsSubmenu, getAlarmEntryName(i), initManageAlarm, manageAlarms, &font4pt7b);
            }
        }

        if (data.isSubmenu && data.currentSubmenu == alarmsSubmenu->entries)
        {
            data.submenuCount = alarmsSubmenu->count;
            showMenu();
        }
    }
}

void initManageAlarm()
{
    alarmCurrentState = 0;
    isEditingAlarm = false;
    AlarmMenuUpdate = true;
    inDaySelectionMode = false;
    useButton();
}

static unsigned long lastRepeatTime = 0;

void drawSelectableText(const String &label, int16_t x, int16_t y, bool selected, bool editing)
{
    int16_t x1, y1;
    uint16_t w, h;

    oled.getTextBounds(label, x, y, &x1, &y1, &w, &h);

    if (editing)
        oled.drawRect(x1 - 2, y1 - 2, w + 4, h + 4, SSD1327_WHITE);
    else if (selected)
    {
        oled.fillRect(x1 - 2, y1 - 2, w + 4, h + 4, SSD1327_WHITE);
        oled.setTextColor(SSD1327_BLACK, SSD1327_WHITE);
    }
    else
        oled.setTextColor(SSD1327_WHITE, SSD1327_BLACK);

    oled.setCursor(x, y);
    oled.print(label);
}

void drawSelectableBox(int16_t x, int16_t y, int w, int h, bool selected, bool editing)
{
    oled.drawRect(
        x,
        y,
        w,
        h,
        (selected || editing) ? SSD1327_WHITE : SSD1327_BLACK);
}

int drawToggleLabel(
    const String &label,
    int16_t x,
    int16_t y,
    bool enabled,
    bool selected = false)
{
    int16_t x1, y1;
    uint16_t w, h;

    oled.getTextBounds(label, 0, 0, &x1, &y1, &w, &h);

    oled.setTextColor(
        enabled ? SSD1327_WHITE : 0x5,
        enabled ? SSD1327_BLACK : SSD1327_BLACK);

    oled.setCursor(x, y);
    oled.print(label);

    if (selected)
        oled.drawRect(x - 1, y - 9, w + 2, 11, 0x8);
    return w;
}

// Updated handleValueRepeat to accept a redraw callback function
template <typename T, typename R>
void handleValueRepeat(
    uint8_t pin,
    T callback,
    R redrawCallback,
    unsigned long &lastRepeatTime,
    uint16_t repeatDelay = 200)
{
    delay(100);

    while (buttonRead(pin) == BUT_CLICK_STATE)
    {
        if (millis() - lastRepeatTime > repeatDelay)
        {
            callback();
            redrawCallback(); // Trigger screen redraw on each increment/decrement
            lastRepeatTime = millis();
        }

        delay(50);
    }
}

void manageAlarms()
{
    uint8_t alarmIndex = alarmsSubmenu->entries[data.currentButton].text.toInt();
    bool exitAlarm = false;

    inkButtonStates btn = useButton();

    auto redrawDisplay = [&]()
    {
        oled.clearDisplay();

        // ==========================================
        // 1. TOP HEADER: Alarm Name & [DEL] Button
        // ==========================================
        oled.setFont(&DejaVu_LGC_Sans_Bold_9);
        oled.setTextColor(SSD1327_WHITE, SSD1327_BLACK);

        // Alarm Title & Day
        oled.setCursor(2, 12);
        oled.print("Alarm #" + String(alarmIndex));

        oled.setTextColor(12);
        oled.print(" (" + getShortCurrentWeekdayName() + ")");

        // Right-Aligned [DEL] Button in Header
        drawSelectableText(
            "[DEL]",
            96,
            12,
            !inDaySelectionMode && alarmCurrentState == 6,
            isEditingAlarm && alarmCurrentState == 6);

        oled.drawLine(0, 18, 127, 18, 6); // Header Divider

        // ==========================================
        // 2. TIME PICKER (Hours & Minutes)
        // ==========================================
        oled.setFont(&DejaVu_Sans_Bold_16);

        // Hours
        drawSelectableText(
            formatWithLeadingZero(alarms[alarmIndex].hours),
            26,
            40,
            !inDaySelectionMode && alarmCurrentState == 0,
            isEditingAlarm && alarmCurrentState == 0);

        // Colon
        oled.setTextColor(SSD1327_WHITE);
        oled.setCursor(58, 38);
        oled.print(":");

        // Minutes
        drawSelectableText(
            formatWithLeadingZero(alarms[alarmIndex].minutes),
            68,
            40,
            !inDaySelectionMode && alarmCurrentState == 1,
            isEditingAlarm && alarmCurrentState == 1);

        // ==========================================
        // 3. FULL-WIDTH DAY SELECTOR (Spans 0 to 127px)
        // ==========================================
        oled.drawLine(0, 48, 127, 48, 6);

        // Outer Bounding Box when Day Row is Focused
        if (!inDaySelectionMode && alarmCurrentState == 2)
        {
            oled.drawRect(0, 50, 128, 24, SSD1327_WHITE);
        }

        // Distribute 7 Days Evenly Across 128px (Width ~18px per slot)
        oled.setFont(&DejaVu_LGC_Sans_Bold_9);
        const int daySlotWidth = 18;

        for (int i = 0; i < 7; i++)
        {
            int slotX = i * daySlotWidth + 3;
            bool selected = inDaySelectionMode && i == alarmCurrentState;

            drawToggleLabel(
                getShorterWeekdayName(i + 1),
                slotX,
                66,
                alarms[alarmIndex].days[i],
                selected);
        }

        oled.drawLine(0, 76, 127, 76, 6);

        // ==========================================
        // 4. FULL-WIDTH TOGGLES (Active, Sound, Light)
        // ==========================================
        oled.setFont(&DejaVu_LGC_Sans_Bold_9);

        // Line 1: Active Status
        drawSelectableText(
            "Active: " + String(alarms[alarmIndex].enabled ? "ENABLED" : "DISABLED"),
            2,
            91,
            !inDaySelectionMode && alarmCurrentState == 3,
            isEditingAlarm && alarmCurrentState == 3);

        // Line 2: Sound & Light Side-by-Side
        drawSelectableText(
            "Sound: " + String(alarms[alarmIndex].soundOn ? "ON" : "OFF"),
            2,
            108,
            !inDaySelectionMode && alarmCurrentState == 4,
            isEditingAlarm && alarmCurrentState == 4);

        drawSelectableText(
            "Light: " + String(alarms[alarmIndex].lightOn ? "ON" : "OFF"),
            68,
            108,
            !inDaySelectionMode && alarmCurrentState == 5,
            isEditingAlarm && alarmCurrentState == 5);

        // Bottom Decorative Line
        oled.drawLine(0, 116, 127, 116, 6);

        oledMana.display();
    };

    auto updateAlarmValueUp = [&]()
    {
        if (alarmCurrentState == 0)
            alarms[alarmIndex].hours = (alarms[alarmIndex].hours + 1) % 24;
        else if (alarmCurrentState == 1)
            alarms[alarmIndex].minutes = (alarms[alarmIndex].minutes + 1) % 60;

        AlarmMenuUpdate = true;
    };

    auto updateAlarmValueDown = [&]()
    {
        if (alarmCurrentState == 0)
            alarms[alarmIndex].hours = (alarms[alarmIndex].hours + 23) % 24;
        else if (alarmCurrentState == 1)
            alarms[alarmIndex].minutes = (alarms[alarmIndex].minutes + 59) % 60;

        AlarmMenuUpdate = true;
    };

    switch (btn)
    {
    case Up:
        if (inDaySelectionMode)
            alarmCurrentState = (alarmCurrentState + 6) % 7;
        else if (!isEditingAlarm)
            alarmCurrentState = (alarmCurrentState + 6) % 7;
        else
            updateAlarmValueUp();

        AlarmMenuUpdate = true;
        lastRepeatTime = millis();
        break;

    case Down:
        if (inDaySelectionMode)
            alarmCurrentState = (alarmCurrentState + 1) % 7;
        else if (!isEditingAlarm)
            alarmCurrentState = (alarmCurrentState + 1) % 7;
        else
            updateAlarmValueDown();

        AlarmMenuUpdate = true;
        lastRepeatTime = millis();
        break;

    case LongUp:
        if (isEditingAlarm)
        {
            handleValueRepeat(
                UP_PIN,
                [&]() { updateAlarmValueUp(); },
                redrawDisplay, // Pass redraw function
                lastRepeatTime);
        }
        break;

    case LongDown:
        if (isEditingAlarm)
        {
            handleValueRepeat(
                DOWN_PIN,
                [&]() { updateAlarmValueDown(); },
                redrawDisplay, // Pass redraw function
                lastRepeatTime);
        }
        break;

    case Menu:
        if (inDaySelectionMode)
        {
            alarms[alarmIndex].days[alarmCurrentState] =
                !alarms[alarmIndex].days[alarmCurrentState];
        }
        else if (!isEditingAlarm)
        {
            switch (alarmCurrentState)
            {
            case 2:
                inDaySelectionMode = true;
                alarmCurrentState = 0;
                break;

            case 3:
                alarms[alarmIndex].enabled = !alarms[alarmIndex].enabled;
                break;

            case 4:
                alarms[alarmIndex].soundOn = !alarms[alarmIndex].soundOn;
                break;

            case 5:
                alarms[alarmIndex].lightOn = !alarms[alarmIndex].lightOn;
                break;

            case 6:
                deleteAlarmStatic(alarmIndex);
                break;

            default:
                isEditingAlarm = true;
                break;
            }
        }
        else
        {
            isEditingAlarm = false;
        }

        AlarmMenuUpdate = true;
        break;

    case Back:
        if (inDaySelectionMode)
        {
            inDaySelectionMode = false;
            alarmCurrentState = 2; // Return focus to Day Row
            AlarmMenuUpdate = true;
        }
        else if (isEditingAlarm)
        {
            isEditingAlarm = false;
            AlarmMenuUpdate = true;
        }
        else
        {
            exitAlarm = true;
        }
        break;

    default:
        break;
    }

    if (AlarmMenuUpdate)
    {
        AlarmMenuUpdate = false;
        redrawDisplay();
    }

    if (useAllButtons() != None)
        lastInputTime = millis();

    if (millis() - lastInputTime > LOOP_FUNCTION_TIMEOUT_MS)
        exitAlarm = true;

    if (exitAlarm && data.currentButton != 0)
    {
        AlarmMenuUpdate = false;
        exitLoopFunction = true;
        editCurrentMenuEntry(getAlarmEntryName(alarmIndex));
    }
}

void disableAlarmsIn()
{
    disableAllAlarms();
    refreshAlarmsSubmenu();
}

void enableAlarmsIn()
{
    enableAllAlarms();
    refreshAlarmsSubmenu();
}

void readAlarmsIn()
{
    readAlarms();
    refreshAlarmsSubmenu();
}