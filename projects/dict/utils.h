#pragma once
#include<arpa/inet.h>


void str_input(char* str, size_t size, char* prompt);
int send_packet(int sockfd, int type, size_t size, void* data);
void prompt(int clear, char* str, int line);
int send_packet(int sockfd, int type, size_t size, void* data);