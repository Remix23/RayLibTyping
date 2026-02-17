#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define FONT_SIZE 32
#define PAST_COLOR LIGHTGRAY
#define CURRENT_COLOR GRAY
#define FUTURE_COLOR LIGHTGRAY
#define MAX_LINE_SIZE 10

typedef struct Diagnostics {
    int correct;
    int totalKeystrokes;
} Diagnostics;

typedef struct Text {
    char** lines;
    int lineCount;
} Text;

void DrawStatistics(Diagnostics* d)
{
    DrawText(TextFormat("Correct: %d", d->correct), 10, 30, FONT_SIZE, GREEN);
    DrawText(TextFormat("Total Keystrokes: %d", d->totalKeystrokes), 10, 50, FONT_SIZE, BLUE);
}

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
        printf("%s\n", text->lines[i]);
    }
}

void freeText(Text* text) {
    for (int i = 0; i < text->lineCount; i++) {
        free(text->lines[i]);
    }
    free(text->lines);
    free(text);
}

int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    char message[] = "123\n451\n23456a";
    Text* text = PrepareTextFromString(message);
    printText(text);

    freeText(text);
    return 0;

    InitWindow(screenWidth, screenHeight, "raylib [core] example - basic window");

    SetTargetFPS(120);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    Vector2 mouse = { 0.0f, 0.0f };

    int current = 1;

    // stats:
    Diagnostics d = {0, 0};

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        
        // get the keys:
        int key;
        while ((key = GetCharPressed()) != 0) {
            d.totalKeystrokes++;
            if (key == message[current]) {
                d.correct++;
                current++;
                if (current >= (int)strlen(message)) {
                    current = 0;
                }
            }
        };

        mouse = GetMousePosition();
        Vector2 sizeOfMessage = { (float)MeasureText(message, FONT_SIZE), FONT_SIZE };

        char* m1 = (char*)TextSubtext(message, 0, current);
        char* m2 = message + current + 1;

        float sizeOfFirstPart = (float)MeasureText(m1, FONT_SIZE);
        float sizeOfSecondPart = (float)MeasureText(m2, FONT_SIZE);
        Vector2 posStart = { mouse.x - sizeOfMessage.x / 2, mouse.y - 10 };
        Vector2 currentPos = {posStart.x + sizeOfFirstPart, posStart.y};

        BeginDrawing();

        ClearBackground(RAYWHITE);

        DrawText(m1, posStart.x, posStart.y, FONT_SIZE, PAST_COLOR);
    
        DrawTextCodepoint(GetFontDefault(), message[current], currentPos, FONT_SIZE, CURRENT_COLOR);
        
        DrawText(m2, posStart.x + sizeOfMessage.x - sizeOfSecondPart, currentPos.y, FONT_SIZE, FUTURE_COLOR);

        //----------------------------------------------------------------------------------
        DrawFPS(10, 10);
        EndDrawing();
    }

    // De-Initialization

    CloseWindow();
}


