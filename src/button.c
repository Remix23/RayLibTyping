#include <raylib.h>
#include <stdlib.h>

#include "button.h"
#include "global_declarations.h"
#include "setting.h"

void DrawButton (Button* btn) {
    DrawRectangle(btn->position.x, btn->position.y, btn->size.x, btn->size.y, btn->color);
    DrawText(btn->text, btn->position.x + btn->size.x / 2 - MeasureText(btn->text, 20) / 2, btn->position.y + btn->size.y / 2 - 10, 20, btn->textColor);
}

void FreeButton(Button* btn) {
    free(btn);
}

Button* initButton (Vector2 position, Vector2 size, Color color, char* text, Color textColor, CallbackFunction callback) {
    Button* btn = (Button*)malloc(sizeof(Button));
    btn->position = position;
    btn->size = size;
    btn->color = color;
    btn->text = text;
    btn->textColor = textColor;
    btn->callback = callback;
    return btn;
}