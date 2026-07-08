#include <Arduino.h>
#include "face.h"
#include "app_state.h"
#include "M5StickCPlus2.h"

// ==================================================
// Face Mood / Pattern
// ==================================================

enum FaceMood {
    FACE_MOOD_NEUTRAL,
    FACE_MOOD_AGGRESSIVE,
    FACE_MOOD_HAPPY,
    FACE_MOOD_RELAX
};

enum FacePattern {
    FACE_PATTERN_NEUTRAL_DEFAULT,
    FACE_PATTERN_AGGRESSIVE_DEFAULT,
    FACE_PATTERN_HAPPY_DEFAULT,
    FACE_PATTERN_RELAX_DEFAULT
};

// ==================================================
// Face State
// ==================================================

FaceMood currentFaceMood = FACE_MOOD_NEUTRAL;
FacePattern currentFacePattern = FACE_PATTERN_NEUTRAL_DEFAULT;

bool faceEyesOpen = true;
unsigned long lastBlinkMillis = 0;
unsigned long nextBlinkInterval = 3000;

const unsigned long BLINK_CLOSED_MS = 120;

// 後でランダム表情切り替えに使う予定
unsigned long lastPatternChangeMillis = 0;
unsigned long nextPatternChangeInterval = 5000;

// ==================================================
// Face Utility
// ==================================================

void resetBlinkTimer() {
    lastBlinkMillis = millis();
    nextBlinkInterval = random(2500, 6000);
}

void resetPatternTimer() {
    lastPatternChangeMillis = millis();
    nextPatternChangeInterval = random(4000, 9000);
}

FaceMood getCurrentFaceMood() {
    if (timerState == STATE_TIME_UP) {
        // 仕事完了
        return FACE_MOOD_HAPPY;
    }

    if (timerState == STATE_PAUSED) {
        // 次の仕事待ち
        return FACE_MOOD_NEUTRAL;
    }

    if (currentMode == TIMER_WORK) {
        // 仕事中
        return FACE_MOOD_AGGRESSIVE;
    }

    // 休憩中
    return FACE_MOOD_RELAX;
}

FacePattern chooseFacePattern(FaceMood mood) {
    if (mood == FACE_MOOD_NEUTRAL) {
        return FACE_PATTERN_NEUTRAL_DEFAULT;
    }

    if (mood == FACE_MOOD_AGGRESSIVE) {
        return FACE_PATTERN_AGGRESSIVE_DEFAULT;
    }

    if (mood == FACE_MOOD_HAPPY) {
        return FACE_PATTERN_HAPPY_DEFAULT;
    }

    return FACE_PATTERN_RELAX_DEFAULT;
}

void updateFacePattern() {
    FaceMood newMood = getCurrentFaceMood();

    if (newMood != currentFaceMood) {
        currentFaceMood = newMood;
        currentFacePattern = chooseFacePattern(currentFaceMood);
        resetPatternTimer();
        resetBlinkTimer();
    } else {
        // 今は1 Mood = 1 Patternなので、同じMoodなら固定。
        currentFacePattern = chooseFacePattern(currentFaceMood);
    }
}

bool shouldBlink() {
    // Happy中は表情固定
    if (currentFaceMood == FACE_MOOD_HAPPY) {
        return false;
    }

    // Relaxは眠そうな顔固定にしたいので瞬きしない
    if (currentFaceMood == FACE_MOOD_RELAX) {
        return false;
    }

    return true;
}

void updateBlink() {
    if (!shouldBlink()) {
        faceEyesOpen = true;
        return;
    }

    unsigned long now = millis();

    if (faceEyesOpen) {
        if (now - lastBlinkMillis >= nextBlinkInterval) {
            faceEyesOpen = false;
            lastBlinkMillis = now;
        }
    } else {
        if (now - lastBlinkMillis >= BLINK_CLOSED_MS) {
            faceEyesOpen = true;
            resetBlinkTimer();
        }
    }
}

// ==================================================
// Public Face API
// ==================================================

void initFace() {
    randomSeed(millis());

    currentFaceMood = getCurrentFaceMood();
    currentFacePattern = chooseFacePattern(currentFaceMood);

    faceEyesOpen = true;
    resetBlinkTimer();
    resetPatternTimer();
}

void updateFace() {
    updateFacePattern();
    updateBlink();
}

// ==================================================
// Draw Parts
// ==================================================

void drawDotEyes() {
    StickCP2.Display.fillCircle(47, 78, 5, WHITE);
    StickCP2.Display.fillCircle(88, 78, 5, WHITE);
}

void drawClosedEyes() {
    StickCP2.Display.fillRoundRect(39, 78, 16, 4, 2, WHITE);
    StickCP2.Display.fillRoundRect(80, 78, 16, 4, 2, WHITE);
}

void drawNeutralMouth() {
    // （・＿・）の「＿」を長めにする
    StickCP2.Display.fillRoundRect(52, 124, 32, 4, 2, WHITE);
}

void drawOmegaMouth() {
    // (｀・ω・´) の「ω」
    StickCP2.Display.drawLine(56, 119, 62, 127, WHITE);
    StickCP2.Display.drawLine(62, 127, 68, 119, WHITE);
    StickCP2.Display.drawLine(68, 119, 74, 127, WHITE);
    StickCP2.Display.drawLine(74, 127, 80, 119, WHITE);
}

void drawHappyMouth() {
    // //^v^// の「v」
    StickCP2.Display.drawLine(61, 122, 68, 130, WHITE);
    StickCP2.Display.drawLine(68, 130, 75, 122, WHITE);
}

void drawRelaxMouth() {
    // (-。-) の「。」
    StickCP2.Display.drawCircle(68, 124, 5, WHITE);
}

void drawAggressiveEyebrows() {
    // (｀・ω・´) の眉
    StickCP2.Display.drawLine(36, 61, 56, 68, WHITE);
    StickCP2.Display.drawLine(79, 68, 99, 61, WHITE);
}

void drawHappyEyes() {
    // (*^-^*) の ^ ^
    StickCP2.Display.drawLine(38, 81, 47, 72, WHITE);
    StickCP2.Display.drawLine(47, 72, 56, 81, WHITE);

    StickCP2.Display.drawLine(79, 81, 88, 72, WHITE);
    StickCP2.Display.drawLine(88, 72, 97, 81, WHITE);
}

void drawRelaxEyes() {
    // (-。-) の - -
    StickCP2.Display.fillRoundRect(38, 78, 18, 4, 2, WHITE);
    StickCP2.Display.fillRoundRect(79, 78, 18, 4, 2, WHITE);
}

// ==================================================
// Draw Patterns
// ==================================================

void drawNeutralDefaultFace() {
    // （・＿・）
    if (faceEyesOpen) {
        drawDotEyes();
    } else {
        drawClosedEyes();
    }

    drawNeutralMouth();
}

void drawAggressiveDefaultFace() {
    // (｀・ω・´)
    if (faceEyesOpen) {
        drawDotEyes();
    } else {
        drawClosedEyes();
    }

    drawAggressiveEyebrows();
    drawOmegaMouth();
}

void drawHappyDefaultFace() {
    // //^v^//
    drawHappyEyes();
    drawHappyMouth();

    // 左ほっぺ //
    StickCP2.Display.drawLine(28, 103, 23, 113, WHITE);
    StickCP2.Display.drawLine(36, 103, 31, 113, WHITE);

    // 右ほっぺ //
    StickCP2.Display.drawLine(99, 103, 94, 113, WHITE);
    StickCP2.Display.drawLine(107, 103, 102, 113, WHITE);
}

void drawRelaxDefaultFace() {
    // (-。-)
    drawRelaxEyes();
    drawRelaxMouth();

    // 休憩感のために小さい息マーク
    StickCP2.Display.drawCircle(94, 112, 3, WHITE);
    StickCP2.Display.drawCircle(102, 104, 2, WHITE);
}

// ==================================================
// Draw Dispatcher
// ==================================================

void drawFacePattern() {
    if (currentFacePattern == FACE_PATTERN_NEUTRAL_DEFAULT) {
        drawNeutralDefaultFace();
        return;
    }

    if (currentFacePattern == FACE_PATTERN_AGGRESSIVE_DEFAULT) {
        drawAggressiveDefaultFace();
        return;
    }

    if (currentFacePattern == FACE_PATTERN_HAPPY_DEFAULT) {
        drawHappyDefaultFace();
        return;
    }

    drawRelaxDefaultFace();
}

void drawFace() {
    // ここが重要：
    // 画面を描く直前に、必ず現在のタイマー状態から表情を更新する。
    updateFace();

    // 顔エリアだけ黒で消す
    StickCP2.Display.fillRect(0, 0, 135, 180, BLACK);

    drawFacePattern();
}