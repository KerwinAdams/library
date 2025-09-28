#pragma once
#include <mysql/mysql.h>
#include<arpa/inet.h>

typedef enum {
    ERROR = -1,
    CORRECT,

    REG,
    REG_SUCCESS,
    REG_FAILED,

    LOGIN,
    LOGIN_SUCCESS,
    LOGIN_FAILED,

    NEWPWD,
    NEWPWD_SUCCESS,

    SEARCH,
    SEARCH_SUCCESS,
    SEARCH_FAILED,

    HISTORY,
    HISTORY_SUCCESS,
    HISTORY_TRANS_FINISHED,
    HISTORY_FAILED,

    QUIT,
}code_t;

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

void daemonize();
void system_run(int argc, char* argv[]);
int server_init(char* argv[]);
void server_loop(int fd);
void* client_thread(void* arg);
void signal_handler(int sig);
void setup_signal();

int db_init(MYSQL* mysql);
int request_register(MYSQL* mysql, packet_t* packet, user_t* user, int sockfd);
int request_login(MYSQL* mysql, packet_t* packet, user_t* user, int sockfd);
int request_newpwd(MYSQL* mysql, packet_t* packet, user_t* user, int sockfd);
int request_search(MYSQL* mysql, packet_t* packet, int sockfd);
int db_history_add(MYSQL* mysql, history_t history);
int request_history(MYSQL* mysql, packet_t* packet, user_t* user, int sockfd);


int log_msg(char* str1, char* str2, char* str3);
int send_packet(int sockfd, int type, size_t size, void* data);
void reg_mem(void* ptr);