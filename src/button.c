#include <raylib.h>
#include <stdlib.h>

#include "button.h"
#include "setting.h"

void DrawButton (Button* btn) {
    DrawRectangle(btn->position.x, btn->position.y, btn->size.x, btn->size.y, btn->color);
    DrawText(btn->text, btn->position.x + btn->size.x / 2 - MeasureText(btn->text, 20) / 2, btn->position.y + btn->size.y / 2 - FONT_SIZE / 2, FONT_SIZE, btn->textColor);
}

inline void FreeButton(Button* btn) {
    free(btn);
}