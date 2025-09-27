#pragma once
#include <mysql/mysql.h>
#include"utils.h"

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

void system_run(int argc, char* argv[]);

int server_init(char* argv[]);
void server_loop(int fd);
void* client_thread(void* arg);

int db_init(MYSQL* mysql);
int request_register(MYSQL* mysql, packet_t* packet, user_t* user, int sockfd);
int request_login(MYSQL* mysql, packet_t* packet, user_t* user, int sockfd);
int request_newpwd(MYSQL* mysql, packet_t* packet, user_t* user, int sockfd);
int request_search(MYSQL* mysql, packet_t* packet, int sockfd);
int db_history_add(MYSQL* mysql, history_t history);
int request_history(MYSQL* mysql, packet_t* packet, user_t* user, int sockfd);


