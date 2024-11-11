#include "interface.h"

#define BUTTONS_OFFSET 2
#define BUTTON_HEIGHT 10

struct entryMenu
{
    String text;
    void (*function)();
    void (*loopFunction)();
    struct Submenu *submenu;
    bool *boolPtr;
    void (*boolToggleFunction)(bool *);
};

struct Submenu
{
    String name;
    entryMenu *entries;
    int count;
    int maxMenus;
};

// Global menu data
struct menuData
{
    int totalMenus;
    int textSize;
    String menuName;
    int linesThick;
    entryMenu entryList[MAX_MENU_ITEMS];
    int itemsOnPage;
    int currentButton;
    bool isSubmenu;
    entryMenu *currentSubmenu;
    int submenuCount;
};

menuData data = {};
int currentMenuItem = 0;
int pageNumber = 0;
int currentPage = 0;

#define MAX_STACK_SIZE 10
Submenu *menuStack[MAX_STACK_SIZE];
int stackPointer = -1;

unsigned long lastInputTime = 0;
const unsigned long idleTimeout = 300000;

bool startedLoop = false;

bool DownHeld = false;
bool UpHeld = false;

bool checkButtonReleased(int pin, bool &heldState)
{
    bool pressed = digitalRead(pin) == LOW;

    if (!pressed && heldState)
    {
        heldState = false;
        return true;
    }
    else if (pressed && !heldState)
    {
        delay(1);
        if (digitalRead(pin) == LOW)
        {
            heldState = true;
        }
    }

    return false;
}

void resetPreviousItems()
{
    display.clearDisplay();
}

void showMenu()
{
    display.setFont(&DejaVu_LGC_Sans_Bold_10);
    display.clearDisplay();
    display.setTextSize(data.textSize);
    data.itemsOnPage = 4;
    int startingButton = data.currentButton - (data.currentButton % data.itemsOnPage);
    currentPage = data.currentButton / data.itemsOnPage;
    pageNumber = ((data.isSubmenu ? data.submenuCount : data.totalMenus) + data.itemsOnPage - 1) / data.itemsOnPage;

    display.setCursor(0, 0);
    display.setTextColor(SSD1306_WHITE);

    display.setCursor(0, 10);
    display.print(String(currentPage + 1) + "/" + String(pageNumber));

    centerText(data.menuName, 10);

    int y = 20;
    for (int i = startingButton; i < startingButton + data.itemsOnPage && i < (data.isSubmenu ? data.submenuCount : data.totalMenus); i++)
    {
        if (data.currentButton == i)
        {
            display.fillRect(0, y - 8, SCREEN_WIDTH, BUTTON_HEIGHT, SSD1306_WHITE);
            display.setTextColor(SSD1306_BLACK);
        }
        else
        {
            display.setTextColor(SSD1306_WHITE);
        }

        String displayText = (data.isSubmenu ? data.currentSubmenu[i].text : data.entryList[i].text);
        if (data.isSubmenu && data.currentSubmenu[i].boolPtr != nullptr)
        {
            displayText += " " + String(*(data.currentSubmenu[i].boolPtr) ? "True" : "False");
        }
        else if (!data.isSubmenu && data.entryList[i].boolPtr != nullptr)
        {
            displayText += " " + String(*(data.entryList[i].boolPtr) ? "True" : "False");
        }

        display.setCursor(0, y);
        display.print(displayText);

        y += BUTTON_HEIGHT + BUTTONS_OFFSET;
    }

    manager.oledDisplay();
    display.setFont(&DejaVu_LGC_Sans_Bold_10);
    display.setTextColor(SSD1306_WHITE);
}

void initMenu(entryMenu *entryList, int totalMenus, String menuName, int textSize, int linesThick)
{
    int realTotalMenus = 0;
    for (int i = 0; i < totalMenus; i++)
    {
        if (entryList[i].text != "")
        {
            data.entryList[realTotalMenus] = entryList[i];
            realTotalMenus++;
        }
    }

    data.totalMenus = realTotalMenus;
    data.textSize = textSize;
    data.menuName = menuName;
    data.linesThick = linesThick;
    data.currentButton = 0;
    data.itemsOnPage = (textSize == 1) ? 8 : 5;
    data.isSubmenu = false;
    data.currentSubmenu = nullptr;
    data.submenuCount = 0;

    showMenu();
}

void updateLastInputTime()
{
    lastInputTime = millis();
}

bool idleEnabled = false;

void startIdleAnimation()
{
    static bool confirmHeld = false;
    static bool upHeld = false;
    static bool downHeld = false;
    static bool exitHeld = false;

    Serial.println("Main Page started...");
    delay(100);

    while (millis() - lastInputTime > idleTimeout || idleEnabled == true)
    {
        showMainPage();
        if (digitalRead(BUTTON_CONFIRM_PIN) == LOW ||
            digitalRead(BUTTON_UP_PIN) == LOW ||
            digitalRead(BUTTON_DOWN_PIN) == LOW ||
            digitalRead(BUTTON_EXIT_PIN) == LOW)
        {
            updateLastInputTime();
            break;
        }
    }
    idleEnabled = false;
}

void exitSubmenu()
{
    if (data.isSubmenu && stackPointer >= 0)
    {
        stackPointer--;
        data.currentButton = 0;

        if (stackPointer >= 0)
        {
            Submenu *prevSubmenu = menuStack[stackPointer];
            data.currentSubmenu = prevSubmenu->entries;
            data.submenuCount = prevSubmenu->count;
            data.menuName = prevSubmenu->name;
            data.isSubmenu = true;
        }
        else
        {
            data.currentSubmenu = nullptr;
            data.menuName = "Main Menu";
            data.isSubmenu = false;
        }

        showMenu();
        Serial.println("Exited submenu, now in: " + data.menuName);
    }
    else
    {
        Serial.println("No submenu to exit.");
        idleEnabled = true;
        startIdleAnimation();
    }
}

void pushSubmenu(Submenu *submenu)
{
    if (stackPointer < 9)
    {
        stackPointer++;
        menuStack[stackPointer] = submenu;
    }
}

void runLoopFunction(void (*loopFunction)())
{
    static bool exitHeld = false;

    while (true)
    {
        loopFunction();

        if (checkButtonReleased(BUTTON_EXIT_PIN, exitHeld))
        {
            break;
        }

        delay(50);
    }
    showMenu();
}

void handleConfirm()
{
    if (startedLoop == true)
    {
        if (data.isSubmenu && data.currentSubmenu != nullptr)
        {
            entryMenu selectedEntry = data.currentSubmenu[data.currentButton];

            if (selectedEntry.function != nullptr)
            {
                selectedEntry.function();
                delay(1);
            }

            if (selectedEntry.submenu != nullptr)
            {
                pushSubmenu(selectedEntry.submenu);
                data.currentSubmenu = selectedEntry.submenu->entries;
                data.submenuCount = selectedEntry.submenu->count;
                data.menuName = selectedEntry.submenu->name;
                data.isSubmenu = true;
                data.currentButton = 0;
                showMenu();
            }
            else if (selectedEntry.boolToggleFunction != nullptr)
            {
                selectedEntry.boolToggleFunction(selectedEntry.boolPtr);
                showMenu();
            }
            else if (selectedEntry.loopFunction != nullptr)
            {
                runLoopFunction(selectedEntry.loopFunction);
            }
        }
        else if (data.entryList[data.currentButton].function != nullptr)
        {
            data.entryList[data.currentButton].function();

            if (data.entryList[data.currentButton].submenu != nullptr)
            {
                pushSubmenu(data.entryList[data.currentButton].submenu);
                data.currentSubmenu = data.entryList[data.currentButton].submenu->entries;
                data.submenuCount = data.entryList[data.currentButton].submenu->count;
                data.menuName = data.entryList[data.currentButton].submenu->name;
                data.isSubmenu = true;
                data.currentButton = 0;
                showMenu();
            }
        }
        else if (data.entryList[data.currentButton].submenu != nullptr)
        {
            pushSubmenu(data.entryList[data.currentButton].submenu);
            data.currentSubmenu = data.entryList[data.currentButton].submenu->entries;
            data.submenuCount = data.entryList[data.currentButton].submenu->count;
            data.menuName = data.entryList[data.currentButton].submenu->name;
            data.isSubmenu = true;
            data.currentButton = 0;
            showMenu();
        }
        else if (data.entryList[data.currentButton].boolToggleFunction != nullptr)
        {
            data.entryList[data.currentButton].boolToggleFunction(data.entryList[data.currentButton].boolPtr);
            showMenu();
        }
        else if (data.entryList[data.currentButton].loopFunction != nullptr)
        {
            runLoopFunction(data.entryList[data.currentButton].loopFunction);
        }
    }
}

void loopMenu()
{
    static bool upHeld = false;
    static bool downHeld = false;
    static bool confirmHeld = false;
    static bool exitHeld = false;

    if (checkButtonReleased(BUTTON_UP_PIN, upHeld))
    {
        data.currentButton = max(data.currentButton - 1, 0);
        showMenu();
    }

    if (checkButtonReleased(BUTTON_DOWN_PIN, downHeld))
    {
        data.currentButton = min(data.currentButton + 1, (data.isSubmenu ? data.submenuCount - 1 : data.totalMenus - 1));
        showMenu();
    }

    if (checkButtonReleased(BUTTON_CONFIRM_PIN, confirmHeld))
    {
        handleConfirm();
    }

    if (checkButtonReleased(BUTTON_EXIT_PIN, exitHeld))
    {
        exitSubmenu();
    }
}

void addMenuEntry(entryMenu entry)
{
    if (data.isSubmenu && data.currentSubmenu != nullptr)
    {
        Submenu *prevSubmenu = menuStack[stackPointer + 1];

        if (data.submenuCount < prevSubmenu->maxMenus)
        {
            data.currentSubmenu[data.submenuCount++] = entry;
            showMenu();
            Serial.println("Added entry to submenu: " + entry.text);
        }
        else
        {
            Serial.println("Failed to add entry to submenu: Max menu items reached.");
        }
    }
    else
    {
        if (data.totalMenus < MAX_MENU_ITEMS)
        {
            data.entryList[data.totalMenus++] = entry;
            showMenu();
            Serial.println("Added entry to main menu: " + entry.text);
        }
        else
        {
            Serial.println("Failed to add entry to main menu: Max menu items reached.");
        }
    }
}

void removeMenuEntry(int index)
{
    if (data.isSubmenu && data.currentSubmenu != nullptr)
    {
        if (index >= 0 && index < data.submenuCount)
        {
            for (int i = index; i < data.submenuCount - 1; i++)
            {
                data.currentSubmenu[i] = data.currentSubmenu[i + 1];
            }
            data.submenuCount--;
            showMenu();
            Serial.println("Removed entry from submenu at index: " + String(index));
        }
    }
    else
    {
        if (index >= 0 && index < data.totalMenus)
        {
            for (int i = index; i < data.totalMenus - 1; i++)
            {
                data.entryList[i] = data.entryList[i + 1];
            }
            data.totalMenus--;
            showMenu();
            Serial.println("Removed entry from main menu at index: " + String(index));
        }
    }
}

void editCurrentMenuEntry(String newText, void (*newFunction)() = nullptr, void (*newLoopFunction)() = nullptr)
{
    entryMenu *currentEntry;
    if (data.isSubmenu && data.currentSubmenu != nullptr)
    {
        currentEntry = &data.currentSubmenu[data.currentButton];
    }
    else
    {
        currentEntry = &data.entryList[data.currentButton];
    }

    currentEntry->text = newText;

    if (newFunction != nullptr)
        currentEntry->function = newFunction;

    if (newLoopFunction != nullptr)
        currentEntry->loopFunction = newLoopFunction;

    showMenu();
}

bool myBool = false;

void toggleBool(bool *boolPtr)
{
    *boolPtr = !(*boolPtr);
}

entryMenu newButton = {"New Item", nullptr, nullptr, nullptr, &myBool, toggleBool};

void test()
{
    addMenuEntry(newButton);
}

bool menuRunning;

int test1 = 0;
String hour1 = "hour";
void loopExample()
{
}

Submenu *createSubmenu(const String &name, int maxMenus, void (*function)() = nullptr)
{
    Submenu *submenu = new Submenu;
    submenu->name = name;
    submenu->entries = new entryMenu[maxMenus];
    submenu->count = 0;
    submenu->maxMenus = maxMenus;
    return submenu;
}

bool addEntryToSubmenu(Submenu *submenu, const String &text,
                       void (*function)(),
                       void (*loopFunction)() = nullptr,
                       bool *boolPtr = nullptr,
                       void (*boolToggleFunction)(bool *) = nullptr)
{
    if (submenu->count >= submenu->maxMenus)
    {
        Serial.println("Submenu is full. Cannot add more entries.");
        return false;
    }
    submenu->entries[submenu->count].text = text;
    submenu->entries[submenu->count].function = nullptr;
    submenu->entries[submenu->count].loopFunction = loopFunction;
    submenu->entries[submenu->count].submenu = nullptr;
    submenu->entries[submenu->count].boolPtr = boolPtr;
    submenu->entries[submenu->count].boolToggleFunction = boolToggleFunction;
    submenu->count++;
    return true;
}
int currentAlarmIndex = 0;
int currentEntryAlarm = 0;

void deleteAlarm(int index)
{
    alarms[index].exists = false;
    showMenu();
}

void deleteAlarmStatic();
void changeAlarmDay();

void setEntryIndex() {
    currentEntryAlarm = data.currentButton;
    Serial.println("this function has run");
}

Submenu *createAlarmSubmenu(int index, String menuName)
{
    // index--; // what the heck why is this here
    currentAlarmIndex = index; // sure? should not break anything...

    Submenu *alarmSubmenu = createSubmenu(menuName, 3,setEntryIndex);

    bool *isEnabled = &alarms[index].enabled;

    addEntryToSubmenu(alarmSubmenu, "Enabled: ", nullptr, nullptr, isEnabled, toggleBool);
    addEntryToSubmenu(alarmSubmenu, "Change Day", nullptr, changeAlarmDay, nullptr, nullptr);

    addEntryToSubmenu(alarmSubmenu, "Delete", deleteAlarmStatic, nullptr, nullptr, nullptr);

    return alarmSubmenu;
}

Submenu *createAlarmsMenu()
{
    Submenu *alarmsSubmenu = nullptr;
    if (!alarmsSubmenu)
    {
        alarmsSubmenu = createSubmenu("Alarms", MAX_ALARMS + 2); // +2 for the managing menus mhm

        // Add "Add New Alarm" option
        addEntryToSubmenu(alarmsSubmenu, "Add New Alarm", []()
                          {
            addNewAlarm();
            showMenu(); });

        addEntryToSubmenu(alarmsSubmenu, "Save Alarms", []()
                          {
            saveAlarms();
            showMenu(); });
    }
    return alarmsSubmenu;
}

Submenu *alarmsSubmenu = createAlarmsMenu();

void deleteAlarmStatic()
{
    if (currentAlarmIndex >= 0)
    {
        String str = data.menuName;
        int number = str.substring(0).toInt();
        deleteAlarm(number);
        exitSubmenu();

        removeMenuEntry(currentAlarmIndex);

        alarmsSubmenu->count--;

        showMenu();
    }
}

void changeAlarmDay() // kil me
{
    display.fillRoundRect(10, 15, SCREEN_WIDTH - 20, 40, 5, SSD1306_BLACK);
    display.drawRoundRect(10, 15, SCREEN_WIDTH - 20, 40, 5, SSD1306_WHITE);
    display.setFont(&DejaVu_LGC_Sans_15);
    centerText(getWeekdayName(alarms[currentAlarmIndex].day + 1), SCREEN_HEIGHT / 2);

    if (checkButtonReleased(BUTTON_DOWN_PIN, DownHeld) && alarms[currentAlarmIndex].day < 6) // ?
    {
        alarms[currentAlarmIndex].day++;
        delay(5);
    }

    if (checkButtonReleased(BUTTON_UP_PIN, DownHeld) && alarms[currentAlarmIndex].day > 0)
    {
        alarms[currentAlarmIndex].day--;
        delay(5);
    }

    Serial.println(String(currentEntryAlarm)); // change the name pleasee

    manager.oledDisplay();

    data.currentSubmenu->text = String(currentAlarmIndex + 1) + " " + getWeekdayName(alarms[currentAlarmIndex].day + 1);
    alarmsSubmenu->entries[currentEntryAlarm].text = String(currentAlarmIndex + 1) + " " + getWeekdayName(alarms[currentAlarmIndex].day + 1);

    delay(1);
}

void setAlarmIndex()
{
    currentAlarmIndex = data.currentButton;
}

void addNewAlarm()
{
    for (int i = 0; i < MAX_ALARMS; ++i)
    {
        if (!alarms[i].exists)
        {
            alarms[i] = {true, true, 0, 0, 0, false}; // probably should do it instead of letting random data in there
            Serial.println("New alarm added. " + String(i));
            data.currentSubmenu = alarmsSubmenu->entries;
            data.submenuCount = alarmsSubmenu->count;
            String menuName = String(i + 1) + " " + getWeekdayName(alarms[i].day + 1);

            Submenu *alarmSubmenu = createAlarmSubmenu(i, menuName);
            addEntryToSubmenu(alarmsSubmenu, String(i + 1) + " " + getWeekdayName(alarms[i].day + 1), setAlarmIndex, nullptr, nullptr, nullptr);
            alarmsSubmenu->entries[alarmsSubmenu->count - 1].submenu = alarmSubmenu;
            // alarmsSubmenu->name = menuName;
            data.submenuCount++;
            data.currentButton = data.submenuCount - 1;
            delay(1);
            showMenu();
            return;
        }
    }
    Serial.println("Failed to add alarm: Maximum number of alarms reached.");
}

void initAlarmMenus()
{
    for (int i = 0; i < MAX_ALARMS; ++i)
    {
        if (alarms[i].exists)
        {
            alarms[i] = {true, true, 0, 0, 0, false};
            Serial.println("New alarm added. " + String(i));
            data.currentSubmenu = alarmsSubmenu->entries;
            data.submenuCount = alarmsSubmenu->count;
            String menuName = String(i + 1) + " " + getWeekdayName(alarms[i].day + 1);

            Submenu *alarmSubmenu = createAlarmSubmenu(i, menuName);
            addEntryToSubmenu(alarmsSubmenu, String(i + 1) + " " + getWeekdayName(alarms[i].day + 1), setAlarmIndex, nullptr, nullptr, nullptr);
            alarmsSubmenu->entries[alarmsSubmenu->count - 1].submenu = alarmSubmenu;
            // alarmsSubmenu->name = menuName;
            data.submenuCount++;
        }
    }
}

void initMenus()
{
    // Create sub-submenu
    entryMenu *subSubmenuItems = new entryMenu[MAX_MENU_ITEMS]{
        {"SubSubItem 1", nullptr, nullptr, nullptr, nullptr},
        {"SubSubItem 2", nullptr, nullptr, nullptr, nullptr}};

    Submenu *subSubmenu = new Submenu{"SubSubmenu", subSubmenuItems, 2, MAX_MENU_ITEMS};

    // Create submenu
    entryMenu *submenuItems = new entryMenu[MAX_MENU_ITEMS]{
        {"SubItem 1", nullptr, nullptr, subSubmenu, nullptr},
        {"SubItem 2", nullptr, nullptr, nullptr, nullptr}};

    Submenu *submenu = new Submenu{"Submenu", submenuItems, 2, MAX_MENU_ITEMS};

    // Initialize main menu buttons
    entryMenu button0 = {"Text 0", test, nullptr, nullptr, nullptr};
    entryMenu button1 = {"Text 1", nullptr, nullptr, nullptr, nullptr};
    entryMenu button2 = {"Text 2", nullptr, loopExample, nullptr, nullptr};
    entryMenu button3 = {"Submenu", nullptr, nullptr, submenu, nullptr};

    // Initialize new button for the alarms menu
    alarmsSubmenu = createAlarmsMenu();                                                   // Call the function to get the Submenu*
    entryMenu alarmsButton = {"Manage Alarms", nullptr, nullptr, alarmsSubmenu, nullptr}; // Use alarmsSubmenu

    // Initialize the main menu
    initMenu(new entryMenu[6]{button0, button1, button2, button3, alarmsButton, newButton}, 6, "Main Menu", 1, 1);
    initAlarmMenus();
}

void handleMenus()
{
    loopMenu();
    startedLoop = true;
}
