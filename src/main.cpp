#include <Arduino.h>
#include "M5StickCPlus2.h"

// ====================
// Timer Settings
// ====================

// Debug用
const int WORK_TIME = 10;
const int BREAK_TIME = 5;
const int EXTEND_TIME = 5;

// 本番用
// const int WORK_TIME = 25 * 60;
// const int BREAK_TIME = 5 * 60;
// const int EXTEND_TIME = 5 * 60;

// ====================
// State
// ====================

enum TimerMode {
    MODE_WORK,
    MODE_BREAK
};

enum TimerState {
    STATE_PAUSED,
    STATE_RUNNING,
    STATE_TIME_UP
};

TimerMode currentMode = MODE_WORK;
TimerState timerState = STATE_PAUSED;

int remainingSeconds = WORK_TIME;
unsigned long lastTickMillis = 0;

// ====================
// Utility
// ====================

const char* getModeName() {
    if (currentMode == MODE_WORK) {
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

int getTotalSecondsForCurrentMode() {
    if (currentMode == MODE_WORK) {
        return WORK_TIME;
    } else {
        return BREAK_TIME;
    }
}

void resetRemainingTime() {
    remainingSeconds = getTotalSecondsForCurrentMode();
}

// ====================
// Sound
// ====================

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

void playNextSound() {
    StickCP2.Speaker.tone(900, 80);
    delay(100);
    StickCP2.Speaker.tone(1300, 80);
}

void playExtendSound() {
    StickCP2.Speaker.tone(800, 60);
    delay(80);
    StickCP2.Speaker.tone(1000, 60);
}

// ====================
// Debug Display
// ====================

void drawDebugScreen() {
    StickCP2.Display.fillScreen(BLACK);

    int minutes = remainingSeconds / 60;
    int seconds = remainingSeconds % 60;

    StickCP2.Display.setTextSize(2);
    StickCP2.Display.setTextColor(WHITE);

    StickCP2.Display.setCursor(10, 15);
    StickCP2.Display.print("CORE TIMER");

    StickCP2.Display.setCursor(10, 55);
    StickCP2.Display.print("Mode:");
    StickCP2.Display.setCursor(10, 80);

    if (currentMode == MODE_WORK) {
        StickCP2.Display.setTextColor(RED);
    } else {
        StickCP2.Display.setTextColor(GREEN);
    }

    StickCP2.Display.print(getModeName());

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

    StickCP2.Display.setTextColor(WHITE);
    StickCP2.Display.setTextSize(3);
    StickCP2.Display.setCursor(10, 175);
    StickCP2.Display.printf("%02d:%02d", minutes, seconds);

    StickCP2.Display.setTextSize(1);
    StickCP2.Display.setCursor(10, 220);

    if (timerState == STATE_TIME_UP) {
        StickCP2.Display.print("A:Next  B:+Time");
    } else {
        StickCP2.Display.print("A:Start/Pause B:+Time");
    }
}

void debugLog(const char* message) {
    Serial.print("[");
    Serial.print(millis());
    Serial.print("] ");
    Serial.print(message);
    Serial.print(" | mode=");
    Serial.print(getModeName());
    Serial.print(" state=");
    Serial.print(getStateName());
    Serial.print(" remain=");
    Serial.println(remainingSeconds);
}

// ====================
// Core Timer Actions
// ====================

void startTimer() {
    timerState = STATE_RUNNING;
    lastTickMillis = millis();

    playStartSound();
    debugLog("startTimer");
    drawDebugScreen();
}

void pauseTimer() {
    timerState = STATE_PAUSED;

    playPauseSound();
    debugLog("pauseTimer");
    drawDebugScreen();
}

void toggleTimer() {
    if (timerState == STATE_RUNNING) {
        pauseTimer();
    } else if (timerState == STATE_PAUSED) {
        startTimer();
    }
}

void extendTimer() {
    remainingSeconds += EXTEND_TIME;

    if (timerState == STATE_TIME_UP) {
        timerState = STATE_RUNNING;
        lastTickMillis = millis();
    }

    playExtendSound();
    debugLog("extendTimer");
    drawDebugScreen();
}

void switchToNextMode() {
    if (currentMode == MODE_WORK) {
        currentMode = MODE_BREAK;
    } else {
        currentMode = MODE_WORK;
    }

    resetRemainingTime();
    timerState = STATE_PAUSED;

    playNextSound();
    debugLog("switchToNextMode");
    drawDebugScreen();
}

void timeUp() {
    remainingSeconds = 0;
    timerState = STATE_TIME_UP;

    debugLog("timeUp");
    drawDebugScreen();
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
        drawDebugScreen();
    }

    if (remainingSeconds <= 0) {
        timeUp();
    }
}

// ====================
// Button Handling
// ====================

void handleButtons() {
    if (StickCP2.BtnA.wasPressed()) {
        if (timerState == STATE_TIME_UP) {
            switchToNextMode();
        } else {
            toggleTimer();
        }
    }

    if (StickCP2.BtnB.wasPressed()) {
        extendTimer();
    }
}

// ====================
// setup / loop
// ====================

void setup() {
    auto cfg = M5.config();
    StickCP2.begin(cfg);

    Serial.begin(115200);
    delay(500);

    StickCP2.Display.setRotation(0);
    StickCP2.Display.setBrightness(80);

    debugLog("setup");
    drawDebugScreen();
}

void loop() {
    StickCP2.update();

    handleButtons();
    updateTimer();

    delay(20);
}