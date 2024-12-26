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
const unsigned long idleTimeout = 3000;

bool startedLoop = false;

bool DownHeld = false;
bool UpHeld = false;

bool menuRunning = true;

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
    manager.stopScrolling();
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

enum IdleState
{
    IDLE,
    ANIMATING
};

IdleState currentState = IDLE;
unsigned long lastAnimationTime = 0;
const unsigned long animationInterval = 1; // Adjust as needed
bool idleEnabled = true;

void startIdleAnimation()
{
    if (idleEnabled)
    {
        if (currentState == IDLE && millis() - lastInputTime > idleTimeout)
        {
            currentState = ANIMATING;
            Serial.println("Main Page started...");
            lastAnimationTime = millis();
        }

        if (currentState == ANIMATING)
        {
            if (millis() - lastAnimationTime >= animationInterval)
            {
                showMainPage();
                lastAnimationTime = millis();
            }

            if (digitalRead(BUTTON_CONFIRM_PIN) == LOW ||
                digitalRead(BUTTON_UP_PIN) == LOW ||
                digitalRead(BUTTON_DOWN_PIN) == LOW ||
                digitalRead(BUTTON_EXIT_PIN) == LOW)
            {
                updateLastInputTime();
                currentState = IDLE;
            }
        }
    }
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
    static unsigned long lastInputTime = 0;  // Track the last input time
    const unsigned long idleTimeout = 10000; // 10 seconds timeout
    static bool timerActive = false;         // Track if the timer is active

    // Check for button actions regardless of timer state
    if (checkButtonReleased(BUTTON_UP_PIN, upHeld))
    {
        data.currentButton = max(data.currentButton - 1, 0);
        showMenu();
        lastInputTime = millis(); // Reset last input time
        timerActive = false;      // Stop the timer
    }

    if (checkButtonReleased(BUTTON_DOWN_PIN, downHeld))
    {
        data.currentButton = min(data.currentButton + 1, (data.isSubmenu ? data.submenuCount - 1 : data.totalMenus - 1));
        showMenu();
        lastInputTime = millis(); // Reset last input time
        timerActive = false;      // Stop the timer
    }

    if (checkButtonReleased(BUTTON_CONFIRM_PIN, confirmHeld))
    {
        handleConfirm();
        lastInputTime = millis(); // Reset last input time
        timerActive = false;      // Stop the timer
    }

    if (checkButtonReleased(BUTTON_EXIT_PIN, exitHeld))
    {
        exitSubmenu();
        lastInputTime = millis(); // Reset last input time
        timerActive = false;      // Stop the timer
    }

    // If no button is pressed, check for idle timeout
    if (!(digitalRead(BUTTON_CONFIRM_PIN) == LOW ||
          digitalRead(BUTTON_UP_PIN) == LOW ||
          digitalRead(BUTTON_DOWN_PIN) == LOW ||
          digitalRead(BUTTON_EXIT_PIN) == LOW))
    {
        // No button is pressed, check for idle timeout
        if (!timerActive)
        {
            lastInputTime = millis(); // Start the timer
            timerActive = true;
        }
        else
        {
            // If the timer is active, check the elapsed time
            if (millis() - lastInputTime >= idleTimeout)
            {
                // Call startIdleAnimation continuously after 10 seconds of inactivity
                startIdleAnimation();
                menuRunning = false;
            }
        }
    }
    else
    {
        // If any button is pressed, reset the timer and show the menu
        lastInputTime = millis(); // Reset the last input time
        timerActive = false;      // Stop the timer
        menuRunning = true;
        showMenu(); // Show the menu
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
                       void (*function)() = nullptr,
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
    submenu->entries[submenu->count].function = function;
    submenu->entries[submenu->count].loopFunction = loopFunction;
    submenu->entries[submenu->count].submenu = nullptr; // Initialize as no submenu
    submenu->entries[submenu->count].boolPtr = boolPtr;
    submenu->entries[submenu->count].boolToggleFunction = boolToggleFunction;
    submenu->count++;
    return true;
}

uint8_t indexList[MAX_ALARMS];

void deleteAlarm(int index)
{
    alarms[index].exists = false;
    showMenu();
}

void deleteAlarmStatic();
void changeAlarmDay();

Submenu *createAlarmsMenu()
{
    Submenu *alarmsSubmenu = nullptr;
    if (!alarmsSubmenu)
    {
        alarmsSubmenu = createSubmenu("Alarms", MAX_ALARMS + 2); // +2 for the managing menus mhm

        // Add "Add New Alarm" option
        addEntryToSubmenu(alarmsSubmenu, "Add New Alarm", addNewAlarm);

        addEntryToSubmenu(alarmsSubmenu, "Save Alarms", saveAlarms);
    }
    return alarmsSubmenu;
}

Submenu *alarmsSubmenu = createAlarmsMenu();

void manageAlarms()
{
    entryMenu *currentEntry;
    currentEntry = &alarmsSubmenu->entries[data.currentButton];
    String entryName = currentEntry->text;
    String a = String("current alarm entry name: ") + entryName;
    Serial.println(a);

    uint8_t alarmIndex = 0;

    if (entryName.length() >= 2)
    {
        alarmIndex = (entryName[0] - '0') * 10 + (entryName[1] - '0');
    }
    a = String("current alarm index: ") + String(alarmIndex);
    Serial.println(a);
}

void deleteAlarmStatic()
{
}

void addNewAlarm()
{
    for (int i = 0; i < MAX_ALARMS; ++i)
    {
        if (!alarms[i].exists)
        {
            alarms[i] = {true, true, 0, 0, 0, false}; // probably should do it instead of letting random data in there
            alarms[i].exists = true;
            Serial.println("New alarm added. " + String(i));
            data.currentSubmenu = alarmsSubmenu->entries;
            data.submenuCount = alarmsSubmenu->count;

            addEntryToSubmenu(alarmsSubmenu, String(i + 1) + " " + getWeekdayName(alarms[i].day + 1) + " " + String(alarms[i].hours) + ":" + String(alarms[i].minutes), nullptr, manageAlarms, nullptr, nullptr);

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
        Serial.println("Alarm exists: " + String(alarms[i].exists));

        if (alarms[i].exists == true)
        {
            Serial.println("New alarm added. " + String(i));

            addEntryToSubmenu(alarmsSubmenu, String(i + 1) + " " + getWeekdayName(alarms[i].day + 1) + " " + String(alarms[i].hours) + ":" + String(alarms[i].minutes), nullptr, manageAlarms, nullptr, nullptr);

            // alarmsSubmenu->name = menuName;
            delay(1);
            showMenu();
        }
    }
}

void handleMenus()
{
    loopMenu();
    startedLoop = true;
}

void menuTask(void *parameter)
{
    while (true)
    {
        handleMenus();
        vTaskDelay(1); // Adjust delay as needed
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
    entryMenu button1 = {"Text 1", nullptr, nullptr, nullptr, nullptr};
    entryMenu button3 = {"Submenu", nullptr, nullptr, submenu, nullptr};

    // Initialize new button for the alarms menu
    alarmsSubmenu = createAlarmsMenu();                                                   // Call the function to get the Submenu*
    entryMenu alarmsButton = {"Manage Alarms", nullptr, nullptr, alarmsSubmenu, nullptr}; // Use alarmsSubmenu

    // Initialize the main menu
    initMenu(new entryMenu[3]{button1, button3, alarmsButton}, 3, "Main Menu", 1, 1);
    initAlarmMenus();
}
