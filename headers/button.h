#pragma once

#include <raylib.h>

typedef struct Button {
    Vector2 position;
    Vector2 size;
    Color color;
    char* text;
    Color textColor;
    int (*callback)(void);
} Button;

void DrawButton(Button* btn);
inline void FreeButton(Button* btn);