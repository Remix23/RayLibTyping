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

#define MIN(a, b) ((a) < (b) ? (a) : (b))

// TODO: add real time diagnostics (accuracy, wpm, etc)
// TODO: shows whats the correct letter 
// TODO: add zodiak api call

Text* PrepareTextFromString (char* message) {
    Text* text = (Text*)malloc(sizeof(Text));
    text->lineCount = 0;
    int current = 0;
    int numOfLines = 8;

    int messageSize = strlen(message);
    char* line = strtok(message, "\n");

    if (line == message) line = strtok(NULL, "\n");

    text->lines = (char**)malloc(numOfLines * sizeof(char*));

    text->lineSizes = (int*)malloc(numOfLines * sizeof(int));
    text->coloring = (Colors**)malloc(numOfLines * sizeof(Colors*));

    while (current < messageSize) {
        if (message[current] == '\0') break;

        // expand the dynmiac array
        if (text->lineCount >= numOfLines - 1) {
            numOfLines += 3;
            text->lines = (char**)realloc(text->lines, numOfLines * sizeof(char*));
            text->lineSizes = (int*)realloc(text->lineSizes, numOfLines * sizeof(int));
            text->coloring = (Colors**)realloc(text->coloring, numOfLines * sizeof(Colors*));
        }
        // two cases: if there is a new line, or line == NULL
        size_t size = 0;

        char* start = message + current;

        if (line == NULL) {
            size = MIN(MAX_LINE_SIZE, strlen(start));
            char* lastWhitespace = strchr(start + size, ' ');
            if (lastWhitespace != NULL) {
                size += lastWhitespace - (start + size);
                current += 1;
            } else {
                size = strlen(start); // last word
            }
        } else {
            size = MIN(strlen(line), MAX_LINE_SIZE);
        }
        
        // copy the line to the text struct
        text->lines[text->lineCount] = (char*)malloc((size + 1) * sizeof(char));
        strncpy(text->lines[text->lineCount], start, size);
        text->lines[text->lineCount][size] = '\0';
        text->lineSizes[text->lineCount] = size;
        if (line != NULL) {
            line = strtok(NULL, "\n");
            current += 1; // +1 for the new line character
        }
        current += size;

        // initialize the incorrect array for this line
        text->coloring[text->lineCount] = (Colors*)malloc(size * sizeof(Colors));
        memset(text->coloring[text->lineCount], PAST, size * sizeof(Colors));
        
        text->lineCount++;
    }
    return text;
}
void printText(Text* text) {
    for (int i = 0; i < text->lineCount; i++) {
        printf("Line [%d] - size [%d]: %s\n", i, text->lineSizes[i], text->lines[i]);
        printf("Incorrect chars: ");
        for (int j = 0; j < text->lineSizes[i]; j++) {
            printf("%d ", text->coloring[i][j]);
        }
            
        printf("\n");
    }
}

void freeText(Text* text) {
    for (int i = 0; i < text->lineCount; i++) {
        free(text->lines[i]);
    }
    for (int i = 0; i < text->lineCount; i++) {
        free(text->coloring[i]);
    }
    free(text->coloring);
    free(text->lines);
    free(text->lineSizes);
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
    for (int i = 0; i < gameState->text->lineCount; i++) {
        memset(gameState->text->coloring[i], FUTURE, gameState->text->lineSizes[i] * sizeof(Colors));
    }
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
    context->currentLine = 0;
    context->current = 0;

    char buffer[MAX_CHAR_MSG] = ""; // local buffer for the new text
    printf("buffer: %s\n", buffer);
    Get(context->curl, &buffer);
    context->text = PrepareTextFromString(&buffer);

    printf("buffer: %s\n", buffer);

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
    free(gameState->diagnostics);
    freeButtons(gameState->buttons, gameState->buttonCount);
    FreeCurl(gameState->curl);
}

float DrawStyledText (char* text, Colors* colors, Vector2 beginPos, int fontSize, float spacing) {
    float offsetX = 0;
    float currentOffsetX = 0;
    Font f = GetFontDefault();
    float scallingFactor = (float)fontSize / f.baseSize;
    for (int i = 0; text[i] != '\0'; i++) {
        Color color = GET_COLOR(colors[i]);

        if (colors[i] == CURRENT) currentOffsetX = offsetX;

        DrawTextCodepoint(GetFontDefault(), text[i], (Vector2){beginPos.x + offsetX, beginPos.y}, fontSize, color);
        int codepoint = GetGlyphIndex(f, text[i]);
        if (f.glyphs[codepoint].offsetX == 0) {
            offsetX += f.recs[codepoint].width * scallingFactor + spacing;
        } else {
            offsetX += f.glyphs[codepoint].advanceX * scallingFactor + spacing;
        }
    }
    return currentOffsetX;
}

int main(void)
{
    GameState gameState = {0};
    gameState.currentLine = 0;
    gameState.current = 0;
    gameState.state = INIT;
    gameState.diagnostics = (Diagnostics*)malloc(sizeof(Diagnostics));
    memset(gameState.diagnostics, 0, sizeof(Diagnostics));
    gameState.text = PrepareTextFromString("Loading...");

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

    SetTargetFPS(FPS);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------
    gameState.state = MENU;

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
loop_begin:
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
        Colors* coloring = gameState.text->coloring[gameState.currentLine];

        // time(gameState.diagnostics->elepsedTime);
        int key;
        while ((key = GetCharPressed()) != 0) {
            if (gameState.text->startTime == 0) {
                time(&gameState.text->startTime);
            }
            gameState.diagnostics->totalKeystrokes++;
            // move the cursor always forward
            gameState.text->coloring[gameState.currentLine][gameState.current] = MISTAKE;
            if (key == message[gameState.current]) {
                gameState.diagnostics->correct++;
                
                gameState.text->coloring[gameState.currentLine][gameState.current] = PAST;
            }
            gameState.current++;
            if (gameState.current >= (int)strlen(message)) {
                gameState.current = 0;
                gameState.currentLine++;
            }
            if (gameState.currentLine >= gameState.text->lineCount) {
                gameState.state = MENU;
                time(&gameState.diagnostics->elepsedTime);
                gameState.diagnostics->elepsedTime -= gameState.text->startTime;
                goto loop_begin;
            } else {
                gameState.text->coloring[gameState.currentLine][gameState.current] = CURRENT;
            }
        }

        if (IsKeyPressed(KEY_BACKSPACE)) {
            gameState.text->coloring[gameState.currentLine][gameState.current] = FUTURE;
            if (gameState.current > 0) {
                gameState.current--;
            } else if (gameState.currentLine > 0) {
                gameState.currentLine--;
                gameState.current = (int)strlen(gameState.text->lines[gameState.currentLine]) - 1;
            }
            gameState.text->coloring[gameState.currentLine][gameState.current] = CURRENT;
        }
        Vector2 posStart = { GetScreenWidth() / 2 - gameState.longestLine / 2, GetScreenHeight() / 2 - FONT_SIZE / 2 };


        BeginDrawing();

        ClearBackground(BACKGROUD);
        // past lines:
        for (int i = 0; i < gameState.currentLine; i++) {
            int lineHeight = FONT_SIZE * (gameState.currentLine - i);
            DrawText(gameState.text->lines[i], posStart.x, posStart.y - lineHeight, FONT_SIZE - 2, PAST_COLOR);
        }

        float currentOffsetX = DrawStyledText(message, coloring, posStart, FONT_SIZE, 2.0);

        for (int i = gameState.currentLine + 1; i < gameState.text->lineCount; i++) {
            int lineHeight = FONT_SIZE * (i - gameState.currentLine);
            DrawText(gameState.text->lines[i], posStart.x, posStart.y + lineHeight, FONT_SIZE - 2, FUTURE_COLOR);
        }

        // draw underscore for the current char:
        DrawLine(posStart.x + currentOffsetX - 2, posStart.y + FONT_SIZE, posStart.x + currentOffsetX + MeasureText("_", FONT_SIZE) + 2, posStart.y + FONT_SIZE, CURRENT_COLOR);

        //----------------------------------------------------------------------------------
        DrawFPS(10, 10);
        EndDrawing();
    }
    freeGameState(&gameState);
    return 0;
}


