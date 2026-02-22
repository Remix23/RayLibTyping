#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <curl/curl.h>
#include <stdbool.h>

#define API_KEY "AV22my54gIgWdr8CcByIFc75O4ibodJx9oxignXq"

// int InitSocket ();

// void bindHost(int sockfd, const char* hostname, const char* port);
// int makeGetRequest (int sockfd, const char* hostname, char* response, size_t response_size);

void* initCurl (const char* url);
int Get (CURL* curl, char* headers);
void FreeCurl (CURL* curl);