#pragma once
#include<arpa/inet.h>

typedef struct {
    int sockfd;
    struct sockaddr_in addr;
}client_t;

typedef struct {
    int type;
    size_t size;
    char data[0];
}packet_t;

typedef struct {
    char name[32];
    char password[32];
}user_t;

typedef struct {
    char user[32];
    char time[32];
    char word[32];
}history_t;

int log_msg(char* str1, char* str2, char* str3);
void str_input(char* str, size_t size, char* prompt);
int send_packet(int sockfd, int type, size_t size, void* data);
void prompt(int clear, char* str, int line);

