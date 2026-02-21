#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <curl/curl.h>
#include <stdbool.h>

#include "networking.h"
#include "setting.h"

static size_t totalSize = 0;

void* initCurl (const char* url, const char* port) 
{
    CURL *curl = curl_easy_init();

    if (!curl) {
        fprintf(stderr, "Failed to initialize curl\n");
        return NULL;
    }
    curl_easy_setopt(curl, CURLOPT_URL, url);
    return curl;
} 

size_t processJSON (char* contents, size_t size, size_t ntimes, void* userp) {
    totalSize += size * ntimes;
    char* buffer = (char*)userp;

    // remove the json parts:
    char* start = strstr(contents, ":");
    char* end = strrchr(contents, '"');

    if (start != NULL && totalSize < MAX_CHAR_MSG) {
        strncat(buffer, start + 3, end - start - 3);
    }
    return totalSize;
}

int Get (CURL* curl, char* buffer) {
    struct curl_slist *chunk = NULL;
    chunk = curl_slist_append(chunk, "X-Api-Key: AV22my54gIgWdr8CcByIFc75O4ibodJx9oxignXq");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, processJSON);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, buffer);
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        return -1;
    }
    totalSize = 0;
    curl_slist_free_all(chunk);
    return 0;
}

void FreeCurl (CURL* curl) {
    curl_easy_cleanup(curl);
}