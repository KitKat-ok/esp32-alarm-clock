#include "oledManager.h"

// Queue to hold actions
QueueHandle_t actionQueue;
// Mutex to ensure only one action runs at a time
SemaphoreHandle_t actionMutex;

bool displaying = false;
bool waitingToDisplay = false;
bool fading = false;
bool waitingToFadein = false;
bool waitingToFadeout = false;

void oledDisplay()
{
    Action action = OLED_DISPLAY;
    xQueueSend(actionQueue, &action, portMAX_DELAY);
}

void oledFadeout()
{
    Action action = OLED_FADE_OUT;
    xQueueSend(actionQueue, &action, portMAX_DELAY);
}

void oledFadein()
{
    Action action = OLED_FADE_IN;
    xQueueSend(actionQueue, &action, portMAX_DELAY);
}

void oledDisable()
{
    Action action = OLED_DISABLE;
    xQueueSend(actionQueue, &action, portMAX_DELAY);
}

void oledEnable()
{
    Action action = OLED_ENABLE;
    xQueueSend(actionQueue, &action, portMAX_DELAY);
}

void sendOledCustomCommand(uint8_t command)
{
    ActionData actionData;
    actionData.action = OLED_CUSTOM_COMMAND;
    actionData.param1 = command;
    xQueueSend(actionQueue, &actionData, portMAX_DELAY);
}

void oledDisableImplementation()
{

    vTaskDelay(pdMS_TO_TICKS(14));
    display.ssd1306_command(SSD1306_DISPLAYOFF);
    vTaskDelay(pdMS_TO_TICKS(14));
}

void oledEnableImplementation()
{
    vTaskDelay(pdMS_TO_TICKS(14));
    display.ssd1306_command(SSD1306_DISPLAYON);
    vTaskDelay(pdMS_TO_TICKS(14));
}

void startScrollingLeft(uint8_t startPage, uint8_t endPage)
{
    ActionData actionData;
    actionData.action = OLED_SCROLL_LEFT;
    actionData.param1 = startPage;
    actionData.param2 = endPage;
    xQueueSend(actionQueue, &actionData, portMAX_DELAY);
}

void startScrollingRight(uint8_t startPage, uint8_t endPage)
{
    ActionData actionData;
    actionData.action = OLED_SCROLL_RIGHT;
    actionData.param1 = startPage;
    actionData.param2 = endPage;
    xQueueSend(actionQueue, &actionData, portMAX_DELAY);
}

void oledDisplayImplementation()
{
    vTaskDelay(pdMS_TO_TICKS(5));
    display.display();
    vTaskDelay(pdMS_TO_TICKS(5));
}

void oledFadeoutImplementation()
{
    if (fading || displaying)
        return; // Check if already fading or displaying

    fading = true; // Set fading flag to true

    // Fade out
    for (int dim = 150; dim >= 0; dim -= 10)
    {
        display.ssd1306_command(0x81); // Contrast control command
        display.ssd1306_command(dim);  // Set contrast value
        delay(10);
    }

    delay(50);

    for (int dim2 = 34; dim2 >= 0; dim2 -= 17)
    {
        display.ssd1306_command(0xD9); // Pre-charge period command
        display.ssd1306_command(dim2); // Set pre-charge period
        delay(20);
    }

    fading = false; // Reset fading flag
}
void oledFadeinImplementation()
{
    fading = true; // Set fading flag to true

    // Fade out
    for (uint8_t dim = 0; dim <= 160; dim += 10)
    {
        display.ssd1306_command(0x81);
        display.ssd1306_command(dim);
        delay(10);
    }

    delay(50);

    for (uint8_t dim2 = 0; dim2 <= 34; dim2 += 17)
    {
        display.ssd1306_command(0xD9);
        display.ssd1306_command(dim2);
        delay(30);
    }

    fading = false; // Reset fading flag
}

void startScrollLeftImplementation(uint8_t startPage, uint8_t endPage)
{
    display.startscrollleft(startPage, endPage);
}

void startScrollRightImplementation(uint8_t startPage, uint8_t endPage)
{
    display.startscrollright(startPage, endPage);
}

void customCommandImplementation(uint8_t command)
{
    display.ssd1306_command(command);
}

#define DEBOUNCE_TIME_MS 5

unsigned long lastActionTimestamp = 0;

Action lastAction = OLED_NO_ACTION;

void oledManagerTask(void *pvParameters)
{
    ActionData actionData;
    while (true)
    {
        if (xQueueReceive(actionQueue, &actionData, portMAX_DELAY))
        {
            unsigned long currentMillis = millis();

            // Handle actions based on debounce time and state
            if (currentMillis - lastActionTimestamp >= DEBOUNCE_TIME_MS || actionData.action != lastAction)
            {
                xSemaphoreTake(actionMutex, portMAX_DELAY);

                switch (actionData.action)
                {
                case OLED_FADE_IN:
                    if (!fading && !displaying)
                    {
                        Serial.println("Processing OLED Fade In");
                        oledFadeinImplementation();
                    }
                    break;
                case OLED_FADE_OUT:
                    if (!fading && !displaying)
                    {
                        Serial.println("Processing OLED Fade Out");
                        oledFadeoutImplementation();
                    }
                    break;
                case OLED_DISPLAY:
                    if (!fading && !displaying)
                    {
                        Serial.println("Processing OLED Display");
                        vTaskDelay(pdMS_TO_TICKS(5));
                        oledDisplayImplementation();
                    }
                    break;
                case OLED_ENABLE:
                    if (!fading && !displaying)
                    {
                        Serial.println("Processing OLED Enable");
                        oledEnableImplementation();
                    }
                    break;
                case OLED_DISABLE:
                    if (!fading && !displaying)
                    {
                        Serial.println("Processing OLED Disable");
                        oledDisableImplementation();
                    }
                    break;
                case OLED_SCROLL_LEFT:
                    Serial.print("Scrolling left with parameters: ");
                    Serial.print(actionData.param1, HEX);
                    Serial.print(", ");
                    Serial.println(actionData.param2, HEX);
                    startScrollLeftImplementation(actionData.param1, actionData.param2);
                    break;
                case OLED_SCROLL_RIGHT:
                    Serial.print("Scrolling right with parameters: ");
                    Serial.print(actionData.param1, HEX);
                    Serial.print(", ");
                    Serial.println(actionData.param2, HEX);
                    startScrollRightImplementation(actionData.param1, actionData.param2);
                    break;
                case OLED_CUSTOM_COMMAND:
                    // Handle custom command if needed
                    break;
                default:
                    Serial.println("Unknown action");
                    break;
                }

                xSemaphoreGive(actionMutex);

                lastAction = actionData.action;
                lastActionTimestamp = currentMillis;
            }
            else
            {
                Serial.println("Action debounced");
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10)); // Adjust delay as needed
    }
}

void initOledManager()
{
    display.clearDisplay();

    actionQueue = xQueueCreate(1, sizeof(ActionData));
    actionMutex = xSemaphoreCreateMutex();

    xTaskCreatePinnedToCore(
        oledManagerTask, /* Task function. */
        "oledManager",   /* String with name of task. */
        10000,           /* Stack size in words. */
        NULL,            /* Parameter passed as input of the task */
        1,               /* Priority of the task. */
        NULL,            /* Task handle. */
        1                /* Core where the task should run. */
    );
}

void createOledManagerTask()
{
    initOledManager();
}
