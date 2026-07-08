#include <Arduino.h>
#include "M5StickCPlus2.h"

// ===== テスト用 =====
const int WORK_TIME = 10;
const int BREAK_TIME = 5;
const int EXTEND_TIME = 5;

// ===== 本番用 =====
// const int WORK_TIME = 25 * 60;
// const int BREAK_TIME = 5 * 60;
// const int EXTEND_TIME = 5 * 60;

enum TimerMode {
    WORK,
    BREAK
};

enum AppState {
    PAUSED,
    RUNNING,
    TIME_UP
};

TimerMode currentMode = WORK;
AppState appState = PAUSED;

int remainingSeconds = WORK_TIME;
unsigned long lastTickMillis = 0;

// --------------------
// 色
// --------------------

uint16_t getThemeColor() {
    if (appState == TIME_UP) {
        return ORANGE;
    }

    if (currentMode == WORK) {
        return RED;
    } else {
        return GREEN;
    }
}

uint16_t getDarkThemeColor() {
    if (appState == TIME_UP) {
        return 0x8200;  // dark orange
    }

    if (currentMode == WORK) {
        return 0x7800;  // dark red
    } else {
        return 0x03E0;  // dark green
    }
}

// --------------------
// 音
// --------------------

void playStartSound() {
    StickCP2.Speaker.tone(1000, 80);
}

void playPauseSound() {
    StickCP2.Speaker.tone(600, 80);
}

void playTimeUpSound() {
    StickCP2.Speaker.tone(1200, 120);
    delay(150);
    StickCP2.Speaker.tone(1600, 120);
    delay(150);
    StickCP2.Speaker.tone(2000, 180);
}

// --------------------
// ｽﾀｯｸﾁｬﾝ風の大きい顔
// --------------------

void drawFaceBase(uint16_t themeColor) {
    // 顔の外枠：画面のほぼ上半分いっぱい
    StickCP2.Display.fillRoundRect(6, 10, 123, 170, 24, themeColor);
    StickCP2.Display.drawRoundRect(6, 10, 123, 170, 24, WHITE);

    // 顔の内側スクリーン
    StickCP2.Display.fillRoundRect(18, 28, 99, 132, 18, BLACK);
    StickCP2.Display.drawRoundRect(18, 28, 99, 132, 18, WHITE);
}

void drawWorkFace() {
    // 集中顔
    StickCP2.Display.fillCircle(48, 82, 9, WHITE);
    StickCP2.Display.fillCircle(88, 82, 9, WHITE);

    StickCP2.Display.fillCircle(51, 84, 4, BLACK);
    StickCP2.Display.fillCircle(85, 84, 4, BLACK);

    // まゆ
    StickCP2.Display.drawLine(36, 65, 58, 58, WHITE);
    StickCP2.Display.drawLine(78, 58, 100, 65, WHITE);

    // 口
    StickCP2.Display.drawLine(54, 128, 82, 128, WHITE);
}

void drawBreakFace() {
    // 休憩中の眠そうな顔
    StickCP2.Display.drawLine(36, 86, 60, 86, WHITE);
    StickCP2.Display.drawLine(76, 86, 100, 86, WHITE);

    // ほっぺ
    StickCP2.Display.fillCircle(38, 108, 6, PINK);
    StickCP2.Display.fillCircle(98, 108, 6, PINK);

    // 口
    StickCP2.Display.drawLine(54, 124, 68, 134, WHITE);
    StickCP2.Display.drawLine(68, 134, 82, 124, WHITE);
}

void drawPausedFace() {
    // 待機中の普通顔
    StickCP2.Display.fillCircle(48, 86, 8, WHITE);
    StickCP2.Display.fillCircle(88, 86, 8, WHITE);

    StickCP2.Display.fillCircle(48, 86, 3, BLACK);
    StickCP2.Display.fillCircle(88, 86, 3, BLACK);

    // にこ口
    StickCP2.Display.drawLine(54, 124, 68, 134, WHITE);
    StickCP2.Display.drawLine(68, 134, 82, 124, WHITE);
}

void drawTimeUpFace() {
    // びっくり顔
    StickCP2.Display.fillCircle(48, 82, 11, WHITE);
    StickCP2.Display.fillCircle(88, 82, 11, WHITE);

    StickCP2.Display.fillCircle(48, 82, 4, BLACK);
    StickCP2.Display.fillCircle(88, 82, 4, BLACK);

    StickCP2.Display.drawCircle(68, 128, 11, WHITE);

    // 汗
    StickCP2.Display.fillTriangle(104, 48, 114, 70, 96, 70, CYAN);
}

void drawStackChanFace() {
    uint16_t themeColor = getThemeColor();

    drawFaceBase(themeColor);

    if (appState == TIME_UP) {
        drawTimeUpFace();
    } else if (appState == PAUSED) {
        drawPausedFace();
    } else if (currentMode == WORK) {
        drawWorkFace();
    } else {
        drawBreakFace();
    }
}

// --------------------
// 残り時間バー
// --------------------

int getTotalSecondsForCurrentMode() {
    if (currentMode == WORK) {
        return WORK_TIME;
    } else {
        return BREAK_TIME;
    }
}

void drawProgressBar() {
    int totalSeconds = getTotalSecondsForCurrentMode();

    if (totalSeconds <= 0) {
        totalSeconds = 1;
    }

    int barX = 10;
    int barY = 205;
    int barW = 115;
    int barH = 18;

    int filledW = (barW * remainingSeconds) / totalSeconds;

    if (filledW < 0) {
        filledW = 0;
    }

    if (filledW > barW) {
        filledW = barW;
    }

    // 枠
    StickCP2.Display.drawRoundRect(barX, barY, barW, barH, 8, WHITE);

    // 残り部分
    if (filledW > 3) {
        StickCP2.Display.fillRoundRect(
            barX + 2,
            barY + 2,
            filledW - 4,
            barH - 4,
            6,
            getThemeColor()
        );
    }

    // 減った部分
    if (filledW < barW - 3) {
        StickCP2.Display.fillRoundRect(
            barX + filledW,
            barY + 2,
            barW - filledW - 2,
            barH - 4,
            6,
            getDarkThemeColor()
        );
    }
}

void drawStatusDot() {
    // 右上の小さい状態ドット
    uint16_t color;

    if (appState == RUNNING) {
        color = YELLOW;
    } else if (appState == PAUSED) {
        color = CYAN;
    } else {
        color = ORANGE;
    }

    StickCP2.Display.fillCircle(118, 22, 5, color);
}

void drawScreen() {
    StickCP2.Display.fillScreen(BLACK);

    drawStackChanFace();
    drawStatusDot();
    drawProgressBar();
}

// --------------------
// タイマー操作
// --------------------

void resetTimeForCurrentMode() {
    if (currentMode == WORK) {
        remainingSeconds = WORK_TIME;
    } else {
        remainingSeconds = BREAK_TIME;
    }
}

void goNextMode() {
    if (currentMode == WORK) {
        currentMode = BREAK;
    } else {
        currentMode = WORK;
    }

    resetTimeForCurrentMode();
    appState = PAUSED;
    drawScreen();
}

void extendTime() {
    remainingSeconds += EXTEND_TIME;

    if (appState == TIME_UP) {
        appState = RUNNING;
        lastTickMillis = millis();
        playStartSound();
    }

    drawScreen();
}

// --------------------
// setup / loop
// --------------------

void setup() {
    auto cfg = M5.config();
    StickCP2.begin(cfg);

    StickCP2.Display.setRotation(0);
    StickCP2.Display.setBrightness(80);

    drawScreen();
}

void loop() {
    StickCP2.update();

    if (StickCP2.BtnA.wasPressed()) {
        if (appState == TIME_UP) {
            goNextMode();
        } else if (appState == RUNNING) {
            appState = PAUSED;
            playPauseSound();
            drawScreen();
        } else {
            appState = RUNNING;
            lastTickMillis = millis();
            playStartSound();
            drawScreen();
        }
    }

    if (StickCP2.BtnB.wasPressed()) {
        extendTime();
    }

    if (appState == RUNNING) {
        unsigned long now = millis();

        if (now - lastTickMillis >= 1000) {
            lastTickMillis = now;

            if (remainingSeconds > 0) {
                remainingSeconds--;
                drawScreen();
            }

            if (remainingSeconds <= 0) {
                remainingSeconds = 0;
                appState = TIME_UP;
                drawScreen();
                playTimeUpSound();
            }
        }
    }

    delay(20);
}