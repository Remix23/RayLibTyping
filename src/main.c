#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "networking.h"
#include <curl/curl.h>
#include <time.h>

#include "button.h"
#include "setting.h"

typedef unsigned int uint;

typedef struct Diagnostics {
    int correct;
    int totalKeystrokes;
    time_t elepsedTime;
} Diagnostics;

typedef struct Text {
    char** lines;
    int lineCount;
    bool isDone;
    time_t startTime;
} Text;

Text* PrepareTextFromString (char* message) {
    Text* text = (Text*)malloc(sizeof(Text));
    text->lineCount = 0;
    text->isDone = false;
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

Text* GetNewText() {
    return NULL;
}

void DrawMenu (Diagnostics* d) {
    int startY = GetScreenHeight() / 2 - 100;
    DrawText(TextFormat("Correct: %d", d->correct), 10, startY, FONT_SIZE, GREEN);
    DrawText(TextFormat("Accuracy: %.2f%%", (float)d->correct / d->totalKeystrokes * 100), 10, startY + FONT_SIZE, FONT_SIZE, BLUE);
    DrawText(TextFormat("Total Keystrokes: %d", d->totalKeystrokes), 10, startY + 2 * FONT_SIZE, FONT_SIZE, BLUE);
    DrawText("Done!", GetScreenWidth() / 2 - MeasureText("Done!", FONT_SIZE) / 2, GetScreenHeight() / 2 - FONT_SIZE / 2, FONT_SIZE, GREEN);
} 


int main(void)
{

    char* message;

    char first_text[] = "123\n451\n23456a";
    Text* text = PrepareTextFromString(first_text);
    printText(text);

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);

    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT  , "Typing test");

    SetTargetFPS(120);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // get longest line -> to reload on text change
    int longestLine = 0;
    for (int i = 0; i < text -> lineCount; i++) {
        int lineSize = (int)MeasureText(text->lines[i], FONT_SIZE);
        if (lineSize > longestLine) {
            longestLine = lineSize;
        }
    }

    int current = 1;
    int currentLine = 0;
    time_t currentTime;
    // stats:
    Diagnostics d = {0, 0};

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        if (text->isDone) {
            BeginDrawing();
            ClearBackground(BACKGROUD);
            DrawMenu(&d);
            EndDrawing();
            continue;
        }
        message = text->lines[currentLine];

        time(&currentTime);

        int msgSize = MeasureText(message, FONT_SIZE);
        int key;
        while ((key = GetCharPressed()) != 0) {
            if (text->startTime == 0) {
                time(&text->startTime);
            }
            d.totalKeystrokes++;
            if (key == message[current]) {
                d.correct++;
                current++;
                if (current >= (int)strlen(message)) {
                    current = 0;
                    currentLine++;
                    if (currentLine >= text->lineCount) {
                        text->isDone = true;
                        time(&d.elepsedTime);
                        d.elepsedTime -= text->startTime;
                        // freeText(text);
                    }
                }
            }
        };

        char* m1 = (char*)TextSubtext(message, 0, current);
        char* m2 = message + current + 1;

        float sizeOfFirstPart = (float)MeasureText(m1, FONT_SIZE);
        float sizeOfSecondPart = (float)MeasureText(m2, FONT_SIZE);
        Vector2 posStart = { GetScreenWidth() / 2 - longestLine / 2, GetScreenHeight() / 2 - FONT_SIZE / 2 };
        Vector2 currentPos = {posStart.x + sizeOfFirstPart + 2, posStart.y};

        BeginDrawing();

        ClearBackground(BACKGROUD);

        // compute the starting y: 

        // past lines:
        for (int i = 0; i < currentLine; i++) {
            int lineHeight = FONT_SIZE * (currentLine - i);
            DrawText(text->lines[i], posStart.x, posStart.y - lineHeight, FONT_SIZE - 2, PAST_COLOR);
        }
        // draw current line;
        DrawText(m1, posStart.x, posStart.y, FONT_SIZE, PAST_COLOR);
    
        DrawTextCodepoint(GetFontDefault(), message[current], currentPos, FONT_SIZE, CURRENT_COLOR);
        
        DrawText(m2, posStart.x + msgSize - sizeOfSecondPart, currentPos.y, FONT_SIZE, FUTURE_COLOR);

        for (int i = currentLine + 1; i < text->lineCount; i++) {
            int lineHeight = FONT_SIZE * (i - currentLine);
            DrawText(text->lines[i], posStart.x, posStart.y + lineHeight, FONT_SIZE - 2, FUTURE_COLOR);
        }

        //----------------------------------------------------------------------------------
        DrawFPS(10, 10);
        EndDrawing();
    }

    // De-Initialization

    freeText(text);
    CloseWindow();
}


