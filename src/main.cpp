#include <Arduino.h>
#include "M5StickCPlus2.h"

// ==================================================
// Presets
// ==================================================

struct TimerPreset {
    const char* name;
    int workSeconds;
    int breakSeconds;
    int sets;
};

TimerPreset presets[] = {
    {"DEBUG", 10, 5, 1},
    {"LIGHT", 15 * 60, 5 * 60, 3},
    {"NORMAL", 25 * 60, 5 * 60, 4},
    {"DEEP", 50 * 60, 10 * 60, 2},
};

const int PRESET_COUNT = sizeof(presets) / sizeof(presets[0]);

int currentPresetIndex = 0;

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

TimerMode currentMode = TIMER_WORK;
TimerState timerState = STATE_PAUSED;

int remainingSeconds = 0;
unsigned long lastTickMillis = 0;

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

UiMode uiMode = UI_MAIN;

int currentMenuIndex = 0;
const int MENU_COUNT = 3;

unsigned long lastUiActionMillis = 0;
const unsigned long MENU_TIMEOUT_MS = 3000;

// ==================================================
// Settings
// ==================================================

bool soundEnabled = true;

// ==================================================
// Utility
// ==================================================

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

// ==================================================
// Sound
// ==================================================

void playTone(int freq, int duration) {
    if (!soundEnabled) {
        return;
    }

    StickCP2.Speaker.tone(freq, duration);
}

void playStartSound() {
    playTone(1000, 80);
}

void playPauseSound() {
    playTone(600, 80);
}

void playTimeUpSound() {
    if (!soundEnabled) {
        return;
    }

    StickCP2.Speaker.tone(1200, 120);
    delay(150);
    StickCP2.Speaker.tone(1600, 120);
    delay(150);
    StickCP2.Speaker.tone(2000, 180);
}

void playNextSound() {
    if (!soundEnabled) {
        return;
    }

    StickCP2.Speaker.tone(900, 80);
    delay(100);
    StickCP2.Speaker.tone(1300, 80);
}

void playMenuSound() {
    playTone(700, 50);
}

void playSelectSound() {
    playTone(1100, 60);
}

// ==================================================
// Debug Display
// ==================================================

void drawDebugScreen() {
    StickCP2.Display.fillScreen(BLACK);

    int minutes = remainingSeconds / 60;
    int seconds = remainingSeconds % 60;

    TimerPreset preset = getCurrentPreset();

    // Title
    StickCP2.Display.setTextSize(2);
    StickCP2.Display.setTextColor(WHITE);
    StickCP2.Display.setCursor(10, 10);
    StickCP2.Display.print("CORE TIMER");

    // Preset
    StickCP2.Display.setTextSize(1);
    StickCP2.Display.setTextColor(WHITE);
    StickCP2.Display.setCursor(10, 38);
    StickCP2.Display.print("Preset: ");
    StickCP2.Display.print(preset.name);
    StickCP2.Display.print(" x");
    StickCP2.Display.print(preset.sets);

    // Mode
    StickCP2.Display.setTextSize(2);
    StickCP2.Display.setTextColor(WHITE);
    StickCP2.Display.setCursor(10, 60);
    StickCP2.Display.print("Mode:");

    StickCP2.Display.setCursor(10, 85);
    if (currentMode == TIMER_WORK) {
        StickCP2.Display.setTextColor(RED);
    } else {
        StickCP2.Display.setTextColor(GREEN);
    }
    StickCP2.Display.print(getModeName());

    // State
    StickCP2.Display.setTextColor(WHITE);
    StickCP2.Display.setCursor(10, 115);
    StickCP2.Display.print("State:");

    StickCP2.Display.setCursor(10, 140);
    if (timerState == STATE_RUNNING) {
        StickCP2.Display.setTextColor(YELLOW);
    } else if (timerState == STATE_TIME_UP) {
        StickCP2.Display.setTextColor(ORANGE);
    } else {
        StickCP2.Display.setTextColor(CYAN);
    }
    StickCP2.Display.print(getStateName());

    // Time
    StickCP2.Display.setTextColor(WHITE);
    StickCP2.Display.setTextSize(3);
    StickCP2.Display.setCursor(10, 172);
    StickCP2.Display.printf("%02d:%02d", minutes, seconds);

    // Sound
    StickCP2.Display.setTextSize(1);
    StickCP2.Display.setTextColor(WHITE);
    StickCP2.Display.setCursor(10, 207);
    StickCP2.Display.print("Sound: ");
    StickCP2.Display.print(soundEnabled ? "ON" : "OFF");

    // Guide
    StickCP2.Display.setCursor(10, 225);
    if (timerState == STATE_TIME_UP) {
        StickCP2.Display.print("A:Next  B:Menu");
    } else {
        StickCP2.Display.print("A:Start/Pause B:Menu");
    }
}

// ==================================================
// Full Screen Menu
// ==================================================

void getPresetText(char* buffer, int bufferSize) {
    TimerPreset preset = getCurrentPreset();

    if (currentPresetIndex == 0) {
        // DEBUGだけ秒表示
        snprintf(
            buffer,
            bufferSize,
            "%ds/%ds",
            preset.workSeconds,
            preset.breakSeconds
        );
    } else {
        // 通常プリセットは分表示
        snprintf(
            buffer,
            bufferSize,
            "%dm/%dm",
            preset.workSeconds / 60,
            preset.breakSeconds / 60
        );
    }
}

void drawMenuRow(
    int row,
    const char* label,
    const char* value,
    bool selected,
    uint16_t valueColor
) {
    int y = 42 + row * 58;

    uint16_t darkGrey = 0x4208;

    if (selected) {
        StickCP2.Display.fillRoundRect(6, y - 6, 123, 50, 8, darkGrey);
        StickCP2.Display.drawRoundRect(6, y - 6, 123, 50, 8, WHITE);

        // 選択カーソル
        StickCP2.Display.fillTriangle(
            14, y + 7,
            14, y + 23,
            25, y + 15,
            YELLOW
        );
    }

    // Label
    StickCP2.Display.setTextSize(2);
    StickCP2.Display.setTextColor(selected ? YELLOW : WHITE);
    StickCP2.Display.setCursor(32, y);
    StickCP2.Display.print(label);

    // Value
    StickCP2.Display.setTextSize(2);
    StickCP2.Display.setTextColor(valueColor);
    StickCP2.Display.setCursor(32, y + 24);
    StickCP2.Display.print(value);
}

void drawMenuOverlay() {
    StickCP2.Display.fillScreen(BLACK);

    // Title
    StickCP2.Display.setTextSize(2);
    StickCP2.Display.setTextColor(WHITE);
    StickCP2.Display.setCursor(42, 10);
    StickCP2.Display.print("MENU");

    char presetText[16];
    getPresetText(presetText, sizeof(presetText));

    const char* modeText = getModeName();
    const char* soundText = soundEnabled ? "ON" : "OFF";

    uint16_t modeColor = currentMode == TIMER_WORK ? RED : GREEN;
    uint16_t presetColor = WHITE;
    uint16_t soundColor = soundEnabled ? GREEN : RED;

    drawMenuRow(
        0,
        "MODE",
        modeText,
        currentMenuIndex == MENU_MODE,
        modeColor
    );

    drawMenuRow(
        1,
        "PRESET",
        presetText,
        currentMenuIndex == MENU_PRESET,
        presetColor
    );

    drawMenuRow(
        2,
        "SOUND",
        soundText,
        currentMenuIndex == MENU_SOUND,
        soundColor
    );
}

void drawScreen() {
    if (uiMode == UI_MENU) {
        drawMenuOverlay();
    } else {
        drawDebugScreen();
    }
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
    Serial.println(soundEnabled ? "ON" : "OFF");
}

// ==================================================
// Core Timer Actions
// ==================================================

void startTimer() {
    timerState = STATE_RUNNING;
    lastTickMillis = millis();

    playStartSound();
    debugLog("startTimer");
    drawScreen();
}

void pauseTimer() {
    timerState = STATE_PAUSED;

    playPauseSound();
    debugLog("pauseTimer");
    drawScreen();
}

void toggleTimer() {
    if (timerState == STATE_RUNNING) {
        pauseTimer();
    } else if (timerState == STATE_PAUSED) {
        startTimer();
    }
}

void switchToNextMode() {
    if (currentMode == TIMER_WORK) {
        currentMode = TIMER_BREAK;
    } else {
        currentMode = TIMER_WORK;
    }

    resetRemainingTime();
    timerState = STATE_PAUSED;

    playNextSound();
    debugLog("switchToNextMode");
    drawScreen();
}

void resetCurrentMode() {
    resetRemainingTime();
    timerState = STATE_PAUSED;

    debugLog("resetCurrentMode");
    drawScreen();
}

void timeUp() {
    remainingSeconds = 0;
    timerState = STATE_TIME_UP;

    debugLog("timeUp");
    drawScreen();
    playTimeUpSound();
}

void updateTimer() {
    if (timerState != STATE_RUNNING) {
        return;
    }

    unsigned long now = millis();

    if (now - lastTickMillis < 1000) {
        return;
    }

    lastTickMillis = now;

    if (remainingSeconds > 0) {
        remainingSeconds--;
        debugLog("tick");
        drawScreen();
    }

    if (remainingSeconds <= 0) {
        timeUp();
    }
}

// ==================================================
// Menu Actions
// ==================================================

void nextMenuItem() {
    currentMenuIndex++;

    if (currentMenuIndex >= MENU_COUNT) {
        currentMenuIndex = 0;
    }

    refreshMenuTimeout();
    playMenuSound();
    debugLog("nextMenuItem");
    drawScreen();
}

void nextPreset() {
    currentPresetIndex++;

    if (currentPresetIndex >= PRESET_COUNT) {
        currentPresetIndex = 0;
    }

    resetCurrentMode();

    debugLog("nextPreset");
}

void toggleSound() {
    soundEnabled = !soundEnabled;

    debugLog("toggleSound");
    drawScreen();

    // OFFにした直後は当然鳴らない
    // ONにした直後だけ確認音を鳴らす
    if (soundEnabled) {
        playSelectSound();
    }
}

void executeMenuItem() {
    refreshMenuTimeout();

    if (currentMenuIndex == MENU_MODE) {
        switchToNextMode();
    } else if (currentMenuIndex == MENU_PRESET) {
        nextPreset();
    } else if (currentMenuIndex == MENU_SOUND) {
        toggleSound();
    }

    playSelectSound();
    drawScreen();
}

// ==================================================
// Button Handling
// ==================================================

void handleMainButtons() {
    if (StickCP2.BtnA.wasPressed()) {
        if (timerState == STATE_TIME_UP) {
            switchToNextMode();
        } else {
            toggleTimer();
        }
    }

    if (StickCP2.BtnB.wasPressed()) {
        openMenu();
        playMenuSound();
        debugLog("openMenu");
        drawScreen();
    }
}

void handleMenuButtons() {
    if (StickCP2.BtnA.wasPressed()) {
        executeMenuItem();
    }

    if (StickCP2.BtnB.wasPressed()) {
        nextMenuItem();
    }
}

void handleButtons() {
    if (uiMode == UI_MENU) {
        handleMenuButtons();
    } else {
        handleMainButtons();
    }
}

void updateMenuTimeout() {
    if (uiMode != UI_MENU) {
        return;
    }

    unsigned long now = millis();

    if (now - lastUiActionMillis >= MENU_TIMEOUT_MS) {
        closeMenu();
        debugLog("closeMenu timeout");
        drawScreen();
    }
}

// ==================================================
// setup / loop
// ==================================================

void setup() {
    auto cfg = M5.config();
    StickCP2.begin(cfg);

    Serial.begin(115200);
    delay(500);

    StickCP2.Display.setRotation(0);
    StickCP2.Display.setBrightness(80);

    resetRemainingTime();

    debugLog("setup");
    drawScreen();
}

void loop() {
    StickCP2.update();

    handleButtons();
    updateTimer();
    updateMenuTimeout();

    delay(20);
}