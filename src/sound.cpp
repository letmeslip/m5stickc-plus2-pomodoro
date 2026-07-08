#include <Arduino.h>
#include "sound.h"
#include "app_state.h"
#include "M5StickCPlus2.h"

// ==================================================
// Basic Sound
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

void playTamagotchiAlarmPattern() {
    if (!soundEnabled) {
        return;
    }

    // たまごっち風: 高めで短いピコピコ音
    StickCP2.Speaker.tone(1800, 70);
    delay(90);
    StickCP2.Speaker.tone(2200, 70);
    delay(90);
    StickCP2.Speaker.tone(2600, 100);
}

// ==================================================
// Alarm
// ==================================================

void startAlarm() {
    if (!soundEnabled) {
        return;
    }

    alarmActive = true;
    alarmStartedMillis = millis();
    lastAlarmMillis = 0;
}

void stopAlarm() {
    alarmActive = false;
}

void updateAlarm() {
    if (!alarmActive) {
        return;
    }

    if (!soundEnabled) {
        stopAlarm();
        return;
    }

    unsigned long now = millis();

    if (now - alarmStartedMillis >= ALARM_TIMEOUT_MS) {
        stopAlarm();
        return;
    }

    if (lastAlarmMillis == 0 || now - lastAlarmMillis >= ALARM_INTERVAL_MS) {
        lastAlarmMillis = now;
        playTamagotchiAlarmPattern();
    }
}