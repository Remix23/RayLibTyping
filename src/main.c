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

void restartGame (void *context) {
    printf("Restarting game...\n");
}

void newText (void *context) {
    printf("Getting new text...\n");
}

void checkButtons (Button** buttons, int buttonCount) {
    Vector2 mousePoint = GetMousePosition();
    for (int i = 0; i < buttonCount; i++) {
        Button* btn = buttons[i];
        if (CheckCollisionPointRec(mousePoint, (Rectangle){btn->position.x, btn->position.y, btn->size.x, btn->size.y})) {
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                btn->callback(NULL);
            }
        }
    }
}

void freeButtons(Button** buttons, int buttonCount) {
    for (int i = 0; i < buttonCount; i++) {
        free(buttons[i]);
    }
}

void DrawButtons(Button** buttons, int buttonCount) {
    for (int i = 0; i < buttonCount; i++) {
        DrawButton(buttons[i]);
    }
}

int main(void)
{
    char* message = NULL;

    char* buffer = (char*)malloc(MAX_CHAR_MSG * sizeof(char));
    buffer[0] = '\0'; // Initialize buffer to an empty string

    CURL* curl = initCurl("https://api.api-ninjas.com/v1/facts", "443");

    Get(curl, buffer);

    Text* t = PrepareTextFromString(buffer);

    printText(t);

    freeText(t);
    free(buffer);
    FreeCurl(curl);

    return 0;

    char first_text[] = "123\n451\n23456a";
    Text* text = PrepareTextFromString(first_text);

    // button init:
    Button* btn_restart = initButton((Vector2){WINDOW_WIDTH - 120, 10}, (Vector2){100, 30}, GRAY, "Restart", WHITE, &restartGame);
    Button* btn_again = initButton((Vector2){WINDOW_WIDTH - 120, 50}, (Vector2){100, 30}, GRAY, "New Text", WHITE, &newText);

    Button* buttons[] = {btn_restart, btn_again};

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

            DrawButtons(buttons, 2);
            checkButtons(buttons, 2);
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
    freeText(text);
    freeButtons(buttons, 2);
    CloseWindow();
    FreeCurl(curl);
    return 0;
}


