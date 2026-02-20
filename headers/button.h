#pragma once

#include <raylib.h>

typedef struct Button {
    Vector2 position;
    Vector2 size;
    Color color;
    char* text;
    Color textColor;
    void (*callback)(void *context);
} Button;

void DrawButton(Button* btn);
Button* initButton (Vector2 position, Vector2 size, Color color, char* text, Color textColor, void (*callback)(void *context));
inline void FreeButton(Button* btn);
