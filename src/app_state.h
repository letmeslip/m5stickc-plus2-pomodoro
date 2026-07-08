#pragma once

#include <Arduino.h>

// ==================================================
// Presets
// ==================================================

struct TimerPreset {
    const char* name;
    int workSeconds;
    int breakSeconds;
    int sets;
};

extern TimerPreset presets[];
extern const int PRESET_COUNT;
extern int currentPresetIndex;

// ==================================================
// Timer State
// ==================================================

enum TimerMode {
    TIMER_WORK,
    TIMER_BREAK
};

enum TimerState {
    STATE_PAUSED,
    STATE_RUNNING,
    STATE_TIME_UP
};

extern TimerMode currentMode;
extern TimerState timerState;

extern int remainingSeconds;
extern unsigned long lastTickMillis;

// ==================================================
// UI State
// ==================================================

enum UiMode {
    UI_MAIN,
    UI_MENU
};

enum MenuItem {
    MENU_MODE,
    MENU_PRESET,
    MENU_SOUND
};

extern UiMode uiMode;

extern int currentMenuIndex;
extern const int MENU_COUNT;

extern unsigned long lastUiActionMillis;
extern const unsigned long MENU_TIMEOUT_MS;

// ==================================================
// Settings
// ==================================================

extern bool soundEnabled;

// ==================================================
// Alarm State
// ==================================================

extern bool alarmActive;
extern unsigned long alarmStartedMillis;
extern unsigned long lastAlarmMillis;

extern const unsigned long ALARM_TIMEOUT_MS;
extern const unsigned long ALARM_INTERVAL_MS;

// ==================================================
// Utility
// ==================================================

void initAppState();

TimerPreset getCurrentPreset();

const char* getModeName();
const char* getStateName();

int getWorkSeconds();
int getBreakSeconds();
int getTotalSecondsForCurrentMode();

void resetRemainingTime();

void refreshMenuTimeout();
void openMenu();
void closeMenu();

uint16_t getThemeColor();

void debugLog(const char* message);