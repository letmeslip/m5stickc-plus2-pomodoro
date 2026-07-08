#include "app_state.h"
#include "M5StickCPlus2.h"

// ==================================================
// Presets
// ==================================================

TimerPreset presets[] = {
    {"DEBUG", 10, 5, 1},
    {"LIGHT", 15 * 60, 5 * 60, 3},
    {"NORMAL", 25 * 60, 5 * 60, 4},
    {"DEEP", 50 * 60, 10 * 60, 2},
};

// C++では const はそのままだと別ファイルから見えにくいので extern を付ける
extern const int PRESET_COUNT = sizeof(presets) / sizeof(presets[0]);

int currentPresetIndex = 0;

// ==================================================
// Timer State
// ==================================================

TimerMode currentMode = TIMER_WORK;
TimerState timerState = STATE_PAUSED;

int remainingSeconds = 0;
unsigned long lastTickMillis = 0;

// ==================================================
// UI State
// ==================================================

UiMode uiMode = UI_MAIN;

int currentMenuIndex = 0;
extern const int MENU_COUNT = 3;

unsigned long lastUiActionMillis = 0;
extern const unsigned long MENU_TIMEOUT_MS = 3000;

// ==================================================
// Settings
// ==================================================

bool soundEnabled = true;

// ==================================================
// Alarm State
// ==================================================

bool alarmActive = false;
unsigned long alarmStartedMillis = 0;
unsigned long lastAlarmMillis = 0;

extern const unsigned long ALARM_TIMEOUT_MS = 60 * 1000;
extern const unsigned long ALARM_INTERVAL_MS = 1400;

// ==================================================
// Utility
// ==================================================

void initAppState() {
    resetRemainingTime();
}

TimerPreset getCurrentPreset() {
    return presets[currentPresetIndex];
}

const char* getModeName() {
    if (currentMode == TIMER_WORK) {
        return "WORK";
    } else {
        return "BREAK";
    }
}

const char* getStateName() {
    if (timerState == STATE_RUNNING) {
        return "RUNNING";
    } else if (timerState == STATE_PAUSED) {
        return "PAUSED";
    } else {
        return "TIME_UP";
    }
}

int getWorkSeconds() {
    return presets[currentPresetIndex].workSeconds;
}

int getBreakSeconds() {
    return presets[currentPresetIndex].breakSeconds;
}

int getTotalSecondsForCurrentMode() {
    if (currentMode == TIMER_WORK) {
        return getWorkSeconds();
    } else {
        return getBreakSeconds();
    }
}

void resetRemainingTime() {
    remainingSeconds = getTotalSecondsForCurrentMode();
}

void refreshMenuTimeout() {
    lastUiActionMillis = millis();
}

void openMenu() {
    uiMode = UI_MENU;
    currentMenuIndex = MENU_MODE;
    refreshMenuTimeout();
}

void closeMenu() {
    uiMode = UI_MAIN;
}

uint16_t getThemeColor() {
    if (timerState == STATE_TIME_UP) {
        return RED;
    }

    if (timerState == STATE_PAUSED) {
        return BLUE;
    }

    if (currentMode == TIMER_WORK) {
        return ORANGE;
    }

    return GREEN;
}

void debugLog(const char* message) {
    Serial.print("[");
    Serial.print(millis());
    Serial.print("] ");
    Serial.print(message);
    Serial.print(" | preset=");
    Serial.print(getCurrentPreset().name);
    Serial.print(" mode=");
    Serial.print(getModeName());
    Serial.print(" state=");
    Serial.print(getStateName());
    Serial.print(" remain=");
    Serial.print(remainingSeconds);
    Serial.print(" sound=");
    Serial.print(soundEnabled ? "ON" : "OFF");
    Serial.print(" alarm=");
    Serial.println(alarmActive ? "ON" : "OFF");
}