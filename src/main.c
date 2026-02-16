#include <raylib.h>
#include <string.h>
#include <stdbool.h>

#define FONT_SIZE 32
#define PAST_COLOR LIGHTGRAY
#define CURRENT_COLOR GRAY
#define FUTURE_COLOR LIGHTGRAY
#define LINE_SIZE 30

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
    int index = 0;
    char* line = strtok(message, "\n");
    while (line != NULL) {
        text->lines = (char**)realloc(text->lines, sizeof(char*) * (text->lineCount + 1));
        text->lines[text->lineCount] = line;
        text->lineCount++;
        line = strtok(NULL, "\n");
    }
    return text;
}

int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [core] example - basic window");

    SetTargetFPS(120);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    char *message = "Congrats! You created your first window!\nThis is a multiline text\nusing raylib.";

    

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


