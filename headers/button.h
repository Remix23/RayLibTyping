#pragma once

#include <raylib.h>
#include "global_declarations.h"
typedef struct Button {
    Vector2 position;
    Vector2 size;
    Color color;
    char* text;
    Color textColor;
    CallbackFunction callback;
} Button;

void DrawButton(Button* btn);
Button* initButton (Vector2 position, Vector2 size, Color color, char* text, Color textColor, CallbackFunction callback);
void FreeButton(Button* btn);
