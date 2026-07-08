#include <Arduino.h>
#include "M5StickCPlus2.h"

#include "app_state.h"
#include "sound.h"
#include "face.h"

// ==================================================
// Main Display: Simple HP Gauge Bar
// ==================================================

void drawProgressBar() {
    int totalSeconds = getTotalSecondsForCurrentMode();

    if (totalSeconds <= 0) {
        totalSeconds = 1;
    }

    int barX = 10;
    int barY = 205;
    int barW = 115;
    int barH = 18;

    int innerX = barX + 3;
    int innerY = barY + 3;
    int innerW = barW - 6;
    int innerH = barH - 6;

    int filledW = 0;

    if (timerState == STATE_PAUSED) {
        // 次の仕事開始待ち：青MAX
        filledW = innerW;
    } else if (timerState == STATE_TIME_UP) {
        // 仕事終了：赤MAX
        filledW = innerW;
    } else if (currentMode == TIMER_BREAK) {
        // 休憩中：回復ゲージ。左から右に増える。
        int elapsedSeconds = totalSeconds - remainingSeconds;
        filledW = (innerW * elapsedSeconds) / totalSeconds;
    } else {
        // 仕事中：消耗ゲージ。右から削れる。
        filledW = (innerW * remainingSeconds) / totalSeconds;
    }

    if (filledW < 0) {
        filledW = 0;
    }

    if (filledW > innerW) {
        filledW = innerW;
    }

    // 枠だけ描く。背景は塗らない。
    StickCP2.Display.drawRoundRect(barX, barY, barW, barH, 8, WHITE);

    // 中身を黒で消す。
    StickCP2.Display.fillRoundRect(innerX, innerY, innerW, innerH, 5, BLACK);

    // 色付き部分だけ描く。
    if (filledW > 0) {
        StickCP2.Display.fillRoundRect(
            innerX,
            innerY,
            filledW,
            innerH,
            5,
            getThemeColor()
        );
    }
}

void drawMainScreen() {
    StickCP2.Display.fillScreen(BLACK);

    drawFace();
    drawProgressBar();
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

    uint16_t modeColor = currentMode == TIMER_WORK ? ORANGE : GREEN;
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
        drawMainScreen();
    }
}

// ==================================================
// Core Timer Actions
// ==================================================

void startTimer() {
    stopAlarm();

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
    stopAlarm();

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
    stopAlarm();

    resetRemainingTime();
    timerState = STATE_PAUSED;

    debugLog("resetCurrentMode");
    drawScreen();
}

void startBreakImmediately() {
    stopAlarm();

    currentMode = TIMER_BREAK;
    resetRemainingTime();

    timerState = STATE_RUNNING;
    lastTickMillis = millis();

    playStartSound();
    debugLog("startBreakImmediately");
    drawScreen();
}

void prepareNextWork() {
    currentMode = TIMER_WORK;
    resetRemainingTime();

    timerState = STATE_PAUSED;

    debugLog("prepareNextWork");
    drawScreen();
    startAlarm();
}

void timeUp() {
    remainingSeconds = 0;

    if (currentMode == TIMER_WORK) {
        // 仕事終了：赤MAXで止める。
        // Aを押すと休憩が即開始する。
        timerState = STATE_TIME_UP;

        debugLog("workTimeUp");
        drawScreen();
        startAlarm();
    } else {
        // 休憩終了：青MAXに戻して、次の仕事開始待ち。
        prepareNextWork();
    }
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
    stopAlarm();

    currentPresetIndex++;

    if (currentPresetIndex >= PRESET_COUNT) {
        currentPresetIndex = 0;
    }

    resetCurrentMode();

    debugLog("nextPreset");
}

void toggleSound() {
    soundEnabled = !soundEnabled;

    if (!soundEnabled) {
        stopAlarm();
    }

    debugLog("toggleSound");
    drawScreen();

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
        stopAlarm();

        if (timerState == STATE_TIME_UP) {
            // 仕事終了後の赤状態から、休憩を即開始する。
            startBreakImmediately();
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

    initAppState();

    debugLog("setup");
    drawScreen();
}

void loop() {
    StickCP2.update();

    handleButtons();
    updateTimer();
    updateMenuTimeout();
    updateAlarm();

    delay(20);
}