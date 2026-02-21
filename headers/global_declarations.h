#pragma once

#include <stdbool.h>
#include <time.h>
#include <curl/curl.h>

typedef unsigned int uint; 

typedef struct {
    int correct;
    int totalKeystrokes;
    time_t elepsedTime;
} Diagnostics;

typedef struct Text {
    char** lines;
    bool** incorrect;
    int* lineSizes;
    int lineCount;
    time_t startTime;
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