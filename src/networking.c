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

#include "networking.h"

int InitSocket () {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    return sockfd;
}

void bindHost (int sockfd, const char* hostname, const char* port) {
    struct addrinfo adresses, *results, *p;
    memset(&adresses, 0, sizeof(adresses));
    adresses.ai_family = AF_INET;
    adresses.ai_socktype = SOCK_STREAM;
    adresses.ai_protocol = IPPROTO_TCP;

    int status = getaddrinfo(hostname, port, &adresses, &results);
    if (status != 0) {
        printf("getaddrinfo failed: %s\n", gai_strerror(status));
        exit(EXIT_FAILURE);
    }

    bool connected = false;

    for (p = results; p != NULL; p = p->ai_next) {
        if(connect(sockfd, p -> ai_addr, p -> ai_addrlen) == 0) {
            connected = true;
            break;
        };
    }
    
    if (!connected) {
        perror("socket connection failed");
        freeaddrinfo(results);
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    freeaddrinfo(results);
}

int makeGetRequest (int sockfd,const char* hostname, char* response, size_t response_size) {

    char message[1024];
    snprintf(message, sizeof(message),
        "GET /v1/facts HTTP/1.0\r\n"
        "X-Api-Key: AV22my54gIgWdr8CcByIFc75O4ibodJx9oxignXq\r\n"
        "\r\n");

    ssize_t sent = send(sockfd, message, strlen(message), 0);
    if (sent == -1) {
        perror("send failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    printf("Sent %zd bytes\n", sent);
    ssize_t received;


    while ((received = recv(sockfd, response, response_size - 1, 0)) > 0) {
        response[received] = '\0'; // Null-terminate the received data
        printf("Received %zd bytes\n", received);
    }

    if (received == -1) {
        perror("recv failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    close(sockfd);
    return strlen(response);
}