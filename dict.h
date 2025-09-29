#pragma once
#include <mysql/mysql.h>
#include<arpa/inet.h>

typedef enum {
    ERROR = -1,
    FAILED,
    SUCCESS,

    REG,
    LOGIN,
    NEWPWD,
    SEARCH,
    HISTORY,
    HISTORY_TRANS_FINISHED,
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

//server 函数
void daemonize();
void system_run(int argc, char* argv[]);
int server_init(char* argv[]);
void server_loop(int fd);
void* client_thread(void* arg);
void signal_handler(int sig);
void setup_signal();

//db 函数
int db_init(MYSQL* mysql);
int request_register(MYSQL* mysql, packet_t* packet, user_t* user, int sockfd);
int request_login(MYSQL* mysql, packet_t* packet, user_t* user, int sockfd);
int request_newpwd(MYSQL* mysql, packet_t* packet, user_t* user, int sockfd);
int request_search(MYSQL* mysql, packet_t* packet, int sockfd);
int db_history_add(MYSQL* mysql, history_t history);
int request_history(MYSQL* mysql, packet_t* packet, user_t* user, int sockfd);

//support 函数
int log_msg(const char* str1, const char* str2, const char* str3);
int send_packet(int sockfd, int type, size_t size, void* data);
void reg_mem(void* ptr);