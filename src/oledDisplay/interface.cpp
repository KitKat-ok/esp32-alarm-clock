#include "interface.h"

#define BUTTONS_OFFSET 2
#define BUTTON_HEIGHT 10

struct entryMenu
{
    String text;
    void (*function)();
    void (*loopFunction)(); // Added loop function pointer
    struct Submenu *submenu;
    bool *boolPtr;
    void (*boolToggleFunction)();
};

struct Submenu
{
    String name;
    entryMenu *entries;
    int count;
};

// Global menu data
struct menuData
{
    int totalMenus;
    int textSize;
    String menuName; // Menu name to display (main or submenu name)
    int linesThick;
    entryMenu entryList[MAX_MENU_ITEMS];
    int itemsOnPage;
    int currentButton;
    bool isSubmenu;
    entryMenu *currentSubmenu; // Pointer to current submenu entries
    int submenuCount;
};

menuData data = {};
int currentMenuItem = 0;
int pageNumber = 0;
int currentPage = 0;

#define MAX_STACK_SIZE 10
Submenu *menuStack[MAX_STACK_SIZE];
int stackPointer = -1;

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

    resetPreviousItems();
    showMenu();

    Serial.println("Initialized menu: " + menuName);
}

void exitSubmenu()
{
    if (data.isSubmenu && stackPointer >= 0)
    {
        stackPointer--;
        data.currentButton = 0;

        if (stackPointer >= 0)
        {
            Submenu *prevSubmenu = menuStack[stackPointer + 1];
            data.currentSubmenu = prevSubmenu->entries;
            data.submenuCount = prevSubmenu->count;
            data.menuName = prevSubmenu->name; // Use the stored submenu name
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
    }
}

void loopMenu()
{
    if (digitalRead(BUTTON_UP_PIN) == LOW)
    {
        // Handle button up logic
        delay(200);
    }
    else if (digitalRead(BUTTON_DOWN_PIN) == LOW)
    {
        // Handle button down logic
        delay(200);
    }
    else if (digitalRead(BUTTON_CONFIRM_PIN) == LOW)
    {
        if (data.isSubmenu && data.currentSubmenu != nullptr)
        {
            entryMenu selectedEntry = data.currentSubmenu[data.currentButton];

            // Handle submenu entry selection
            if (selectedEntry.function != nullptr)
            {
                selectedEntry.function();
            }
            else if (selectedEntry.boolToggleFunction != nullptr)
            {
                selectedEntry.boolToggleFunction();
                showMenu();
            }
            else if (selectedEntry.submenu != nullptr)
            {
                // Handle entering submenu
            }
        }
        else if (data.entryList[data.currentButton].function != nullptr)
        {
            data.entryList[data.currentButton].function();
        }
        else if (data.entryList[data.currentButton].boolToggleFunction != nullptr)
        {
            data.entryList[data.currentButton].boolToggleFunction();
            showMenu();
        }
        delay(200);
    }
    else if (digitalRead(BUTTON_EXIT_PIN) == LOW)
    {
        // Handle exit submenu logic
        delay(200);
    }
    else
    {
        // Check if there's a loop function to run
        if (data.isSubmenu && data.currentSubmenu != nullptr)
        {
            entryMenu selectedEntry = data.currentSubmenu[data.currentButton];
            if (selectedEntry.loopFunction != nullptr)
            {
                while (digitalRead(BUTTON_EXIT_PIN) == HIGH)
                {
                    selectedEntry.loopFunction();
                    // Optionally show menu here if needed
                    delay(100); // Adjust delay as needed
                }
            }
        }
        else
        {
            entryMenu selectedEntry = data.entryList[data.currentButton];
            if (selectedEntry.loopFunction != nullptr)
            {
                while (digitalRead(BUTTON_EXIT_PIN) == HIGH)
                {
                    selectedEntry.loopFunction();
                    // Optionally show menu here if needed
                    delay(100); // Adjust delay as needed
                }
            }
        }
    }
}

void addMenuEntry(entryMenu entry)
{
    if (data.isSubmenu && data.currentSubmenu != nullptr)
    {
        if (data.submenuCount < MAX_MENU_ITEMS)
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

bool myBool = false;

void toggleBool()
{
    myBool = !myBool;
}

entryMenu newButton = {"New Item", nullptr, nullptr,nullptr, &myBool, toggleBool};

void test()
{
    addMenuEntry(newButton);
}
bool menuRunning;
void loopExample()
{
    // Example of a loop function that runs continuously
    Serial.println("Loop function is running...");
    delay(50); // Adjust as needed for loop speed
}

void initMenus()
{
    entryMenu *subSubmenuItems = new entryMenu[MAX_MENU_ITEMS]{
        {"SubSubItem 1", nullptr, nullptr, nullptr, nullptr},
        {"SubSubItem 2", nullptr, nullptr, nullptr, nullptr}};

    Submenu *subSubmenu = new Submenu{"SubSubmenu", subSubmenuItems, 2};

    entryMenu *submenuItems = new entryMenu[MAX_MENU_ITEMS]{
        {"SubItem 1", nullptr, nullptr, subSubmenu, nullptr},
        {"SubItem 2", nullptr, nullptr, nullptr, nullptr}};

    Submenu *submenu = new Submenu{"Submenu", submenuItems, 2};

    entryMenu button0 = {"Text 0", test, nullptr, nullptr, nullptr};
    entryMenu button1 = {"Text 1", nullptr, nullptr, nullptr, nullptr};
    entryMenu button2 = {"Text 2", nullptr, loopExample, nullptr, nullptr}; // Added loop function here
    entryMenu button3 = {"Submenu", nullptr, nullptr, submenu, nullptr};

    initMenu(new entryMenu[5]{button0, button1, button2, newButton, button3}, 5, "Main Menu", 1, 1);
}

void handleMenus()
{
    loopMenu();
}
