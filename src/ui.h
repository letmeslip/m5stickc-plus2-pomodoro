#pragma once

void drawProgressBar();
void drawMainScreen();

void getPresetText(char* buffer, int bufferSize);

void drawMenuRow(
    int row,
    const char* label,
    const char* value,
    bool selected,
    unsigned short valueColor
);

void drawMenuOverlay();
void drawScreen();

void handleMainButtons();
void handleMenuButtons();
void handleButtons();

void updateMenuTimeout();