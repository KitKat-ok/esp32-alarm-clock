#pragma once

#include "defines.h"


extern bool buttonsActivated;
extern std::mutex buttMut;
extern inkButtonStates buttonPressed;
extern TaskHandle_t buttonTask;

inkButtonStates useButtonBack();
inkButtonStates useButton();
inkButtonStates useAllButtons();
void loopButtonsTask(void *parameter);

void turnOnButtons();
void initButtonTask();
void deInitButtonTask();
void setButton(inkButtonStates button);
void longButtonCheck(int buttonPin, inkButtonStates normalButton, inkButtonStates longButton);
void turnOnInterrupts();
void resumeButtonTask();
extern inkButtonStates interruptedButton;
bool buttonRead(uint8_t pin); // Wrapper for digitalRead for buttons
