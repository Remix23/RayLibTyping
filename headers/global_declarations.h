#pragma once

#include <stdbool.h>
#include <time.h>
#include <curl/curl.h>

typedef unsigned int uint; 

typedef enum Colors {
    CURRENT, // 0
    PAST, // 1
    MISTAKE, // 
    FUTURE,
} Colors;

#define GET_COLOR(color) \
    (color == CURRENT ? CURRENT_COLOR : \
     color == PAST ? PAST_COLOR : \
     color == MISTAKE ? MISTAKE_COLOR : FUTURE_COLOR)

typedef struct {
    int correct;
    int totalKeystrokes;
    double elepsedTime;
} Diagnostics;

typedef struct Text {
    char** lines;
    Colors** coloring;
    int* lineSizes;
    int lineCount;
    double startTime;
} Text;

typedef struct GameState {
    int currentLine;
    int current;
    int longestLine;
    Text* text;
    Diagnostics* diagnostics;
    struct Button** buttons;
    CURL* curl;
    int buttonCount;
    enum state {
        TYPING, 
        MENU,
        INIT,
    } state;
} GameState;

typedef void (*CallbackFunction)(struct GameState *context);