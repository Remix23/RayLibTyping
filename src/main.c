#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <curl/curl.h>

#include "button.h"
#include "networking.h"
#include "setting.h"
#include "global_declarations.h"

static char buffer[MAX_CHAR_MSG] = "\0";

Text* PrepareTextFromString (char* message) {
    Text* text = (Text*)malloc(sizeof(Text));
    text->lineCount = 0;
    int current = 0;
    int lineSize = 8;
    int messageSize = strlen(message);
    char* line = strtok(message, "\n");
    text->lines = (char**)malloc(lineSize * sizeof(char*));

    while (current < messageSize) {
        if (message[current] == '\0') break;

        // expand the dynmiac array
        if (text->lineCount >= lineSize - 1) {
            lineSize += 8;
            text->lines = (char**)realloc(text->lines, lineSize * sizeof(char*));
        }

        // two cases: if there is a new line, or line == NULL
        if (line != NULL) {
            int size = strlen(line);
            if (size < MAX_LINE_SIZE) {
                // copy the line to the text struct
                text->lines[text->lineCount] = (char*)malloc((size + 1) * sizeof(char));
                strncpy(text->lines[text->lineCount], message + current, size);
                text->lines[text->lineCount][size] = '\0';
                text->lineCount++;
                line = strtok(NULL, "\n");
                current += size + 1;
                continue;
            } 
        }
        // copy the line to the text struct
        text->lines[text->lineCount] = (char*)malloc((MAX_LINE_SIZE + 1) * sizeof(char));
        strncpy(text->lines[text->lineCount], message + current, MAX_LINE_SIZE);
        text->lines[text->lineCount][MAX_LINE_SIZE] = '\0';
        text->lineCount++;
        current += MAX_LINE_SIZE;
    }
    return text;
}
void printText(Text* text) {
    for (int i = 0; i < text->lineCount; i++) {
        printf("Line [%d]: %s\n",i,  text->lines[i]);
    }
}

void freeText(Text* text) {
    for (int i = 0; i < text->lineCount; i++) {
        free(text->lines[i]);
    }
    free(text->lines);
    free(text);
}

void DrawMenu (Diagnostics* d) {
    int startY = GetScreenHeight() / 2 - 100;
    DrawText(TextFormat("Correct: %d", d->correct), 10, startY, FONT_SIZE, GREEN);
    DrawText(TextFormat("Accuracy: %.2f%%", (float)d->correct / d->totalKeystrokes * 100), 10, startY + FONT_SIZE, FONT_SIZE, BLUE);
    DrawText(TextFormat("Total Keystrokes: %d", d->totalKeystrokes), 10, startY + 2 * FONT_SIZE, FONT_SIZE, BLUE);
    DrawText("Done!", GetScreenWidth() / 2 - MeasureText("Done!", FONT_SIZE) / 2, GetScreenHeight() / 2 - FONT_SIZE / 2, FONT_SIZE, GREEN);
} 

void resetTextInfo (GameState* gameState) {
    gameState->currentLine = 0;
    gameState->current = 0;
    buffer[0] = '\0';
}

void resetDiagnostics (Diagnostics* diagnostics) {
    diagnostics->correct = 0;
    diagnostics->totalKeystrokes = 0;
    diagnostics->elepsedTime = 0;
}

void restartGame (struct GameState *context) {
    resetTextInfo(context);
    resetDiagnostics(context->diagnostics);

    context->state = TYPING;
}

void newText (struct GameState *context) {
    freeText(context->text);
    resetTextInfo(context);
    
    Get(context->curl, &buffer);
    context->text = PrepareTextFromString(buffer);

    context->longestLine = 0;
    for (int i = 0; i < context->text->lineCount; i++) {
        int lineSize = (int)MeasureText(context->text->lines[i], FONT_SIZE);
        if (lineSize > context->longestLine) {
            context->longestLine = lineSize;
        }
    }
    
    resetDiagnostics(context->diagnostics);
    context->state = TYPING;
}

void checkButtons (Button** buttons, int buttonCount, GameState* t) {
    Vector2 mousePoint = GetMousePosition();
    for (int i = 0; i < buttonCount; i++) {
        Button* btn = buttons[i];
        if (CheckCollisionPointRec(mousePoint, (Rectangle){btn->position.x, btn->position.y, btn->size.x, btn->size.y})) {
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                btn->callback(t);
            }
        }
    }
}

void freeButtons(Button** buttons, int buttonCount) {
    for (int i = 0; i < buttonCount; i++) {
        FreeButton(buttons[i]);
    }
}

void DrawButtons(Button** buttons, int buttonCount) {
    for (int i = 0; i < buttonCount; i++) {
        DrawButton(buttons[i]);
    }
}

void freeGameState (GameState* gameState) {
    gameState->currentLine = 0;
    gameState->current = 0;
    freeText(gameState->text);
    gameState->text = NULL;
    gameState->diagnostics->correct = 0;
    gameState->diagnostics->totalKeystrokes = 0;
    gameState->diagnostics->elepsedTime = 0;
    freeButtons(gameState->buttons, gameState->buttonCount);
    FreeCurl(gameState->curl);
}

int main(void)
{
    GameState gameState = {0};
    gameState.currentLine = 0;
    gameState.current = 0;
    gameState.state = INIT;
    gameState.diagnostics = (Diagnostics*)malloc(sizeof(Diagnostics));
    memset(gameState.diagnostics, 0, sizeof(Diagnostics));
    gameState.text = PrepareTextFromString("Loadding...");

    gameState.curl = initCurl("https://api.api-ninjas.com/v1/facts", "443");
    // button init:
    Button* btn_restart = initButton((Vector2){WINDOW_WIDTH - 120, 10}, (Vector2){100, 30}, GRAY, "Restart", WHITE, &restartGame);
    Button* btn_again = initButton((Vector2){WINDOW_WIDTH - 120, 50}, (Vector2){100, 30}, GRAY, "New Text", WHITE, &newText);

    gameState.buttons = (Button**)malloc(2 * sizeof(Button*));
    gameState.buttons[0] = btn_restart;
    gameState.buttons[1] = btn_again;
    gameState.buttonCount = 2;

    char* message = NULL;

    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Typing test");

    SetTargetFPS(120);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // get longest line -> to reload on text chan

    gameState.state = MENU;

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        if (gameState.state == MENU) {
            BeginDrawing();
            ClearBackground(BACKGROUD);
            DrawMenu(gameState.diagnostics);

            DrawButtons(gameState.buttons, gameState.buttonCount);
            checkButtons(gameState.buttons, gameState.buttonCount, &gameState);
            EndDrawing();
            continue;
        }
        message = gameState.text->lines[gameState.currentLine];

        // time(gameState.diagnostics->elepsedTime);

        int msgSize = MeasureText(message, FONT_SIZE);
        int key;
        while ((key = GetCharPressed()) != 0) {
            if (gameState.text->startTime == 0) {
                time(&gameState.text->startTime);
            }
            gameState.diagnostics->totalKeystrokes++;
            if (key == message[gameState.current]) {
                gameState.diagnostics->correct++;
                gameState.current++;
                if (gameState.current >= (int)strlen(message)) {
                    gameState.current = 0;
                    gameState.currentLine++;
                    if (gameState.currentLine >= gameState.text->lineCount) {
                        gameState.state = MENU;
                        time(&gameState.diagnostics->elepsedTime);
                        gameState.diagnostics->elepsedTime -= gameState.text->startTime;
                        // freeText(gameState.text);
                    }
                }
            }
        };

        char* m1 = (char*)TextSubtext(message, 0, gameState.current);
        char* m2 = message + gameState.current + 1;

        float sizeOfFirstPart = (float)MeasureText(m1, FONT_SIZE);
        float sizeOfSecondPart = (float)MeasureText(m2, FONT_SIZE);
        Vector2 posStart = { GetScreenWidth() / 2 - gameState.longestLine / 2, GetScreenHeight() / 2 - FONT_SIZE / 2 };
        Vector2 currentPos = {posStart.x + sizeOfFirstPart + 2, posStart.y};

        BeginDrawing();

        ClearBackground(BACKGROUD);

        // compute the starting y: 

        // past lines:
        for (int i = 0; i < gameState.currentLine; i++) {
            int lineHeight = FONT_SIZE * (gameState.currentLine - i);
            DrawText(gameState.text->lines[i], posStart.x, posStart.y - lineHeight, FONT_SIZE - 2, PAST_COLOR);
        }
        // draw current line;
        DrawText(m1, posStart.x, posStart.y, FONT_SIZE, PAST_COLOR);
    
        DrawTextCodepoint(GetFontDefault(), message[gameState.current], currentPos, FONT_SIZE, CURRENT_COLOR);
        
        DrawText(m2, posStart.x + msgSize - sizeOfSecondPart, currentPos.y, FONT_SIZE, FUTURE_COLOR);

        for (int i = gameState.currentLine + 1; i < gameState.text->lineCount; i++) {
            int lineHeight = FONT_SIZE * (i - gameState.currentLine);
            DrawText(gameState.text->lines[i], posStart.x, posStart.y + lineHeight, FONT_SIZE - 2, FUTURE_COLOR);
        }

        //----------------------------------------------------------------------------------
        DrawFPS(10, 10);
        EndDrawing();
    }
    freeGameState(&gameState);
    return 0;
}


