#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/types.h>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<stdio.h>
#include<time.h>
#include"dict.h"
#include"utils.h"

int main(int argc, char* argv[]) {
    system_run(argc, argv);
    return 0;
}


//系统运行函数
void system_run(int argc, char* argv[]) {

    prompt(1, NULL, 0);

    if (argc != 2) {
        printf("usage: %s <port>\n", argv[0]);
        return;
    }

    int sockfd = server_init(argv);
    if (sockfd == -1) {
        log_msg("server init error");
        return;
    }

    log_msg("server start");

    server_loop(sockfd);
    return;
}

//服务器初始化函数
int server_init(char* argv[]) {

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        return -1;
    }

    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[1]));
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    socklen_t addrlen = sizeof(server_addr);

    if (bind(sockfd, (struct sockaddr*)&server_addr, addrlen) < 0) {
        return -1;
    }

    if (listen(sockfd, 5) < 0) {
        return -1;
    }

    return sockfd;
}

//服务器循环函数
void server_loop(int fd) {

    struct sockaddr_in client_addr;
    socklen_t addrlen = sizeof(client_addr);

    while (1) {
        int sockfd = accept(fd, (struct sockaddr*)&client_addr, &addrlen);
        if (sockfd < 0) {
            continue;
        }

        char log_str[128] = { 0 };
        sprintf(log_str, "client %s:%d connected", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        log_msg(log_str);

        client_t* temp = (client_t*)malloc(sizeof(client_t));
        temp->sockfd = sockfd;
        temp->addr = client_addr;

        pthread_t tid;
        if (pthread_create(&tid, NULL, client_thread, temp) != 0) {
            continue;
        }

        if (pthread_detach(tid) != 0) {
            continue;
        }
    }
}

//客户端线程函数
void* client_thread(void* arg) {

    client_t client = *(client_t*)arg;

    char log_str[256] = { 0 };
    char client_info[128] = { 0 };
    sprintf(client_info, "client %s:%d", inet_ntoa(client.addr.sin_addr), ntohs(client.addr.sin_port));


    MYSQL mysql;
    if (db_init(&mysql) == -1) {
        sprintf(log_str, "%s database init error", client_info);
        log_msg(log_str);
        close(client.sockfd);
        free(arg);
        return NULL;
    }

    sprintf(log_str, "%s database init success", client_info);
    log_msg(log_str);

    packet_t packet;
    user_t user;

    while (1) {
        memset(&packet, 0, sizeof(packet_t));
        if (recv(client.sockfd, &packet, sizeof(packet_t), 0) == 0) {
            break;
        }

        if (packet.type == REG) {

            sprintf(log_str, "%s request register", client_info);
            log_msg(log_str);

            int ret = request_register(&mysql, &packet, &user, client.sockfd);
            if (ret == REG_SUCCESS) {
                sprintf(log_str, "%s register success", client_info);
            } else if (ret == REG_FAILED) {
                sprintf(log_str, "%s register failed", client_info);
            } else {
                sprintf(log_str, "%s register error", client_info);
            }
            log_msg(log_str);

        } else if (packet.type == LOGIN) {

            sprintf(log_str, "%s request login", client_info);
            log_msg(log_str);

            int ret = request_login(&mysql, &packet, &user, client.sockfd);
            if (ret == LOGIN_SUCCESS) {
                sprintf(log_str, "%s login success", client_info);
            } else if (ret == LOGIN_FAILED) {
                sprintf(log_str, "%s login failed", client_info);
            } else {
                sprintf(log_str, "%s login error", client_info);
            }
            log_msg(log_str);

        } else if (packet.type == NEWPWD) {
            sprintf(log_str, "%s %s request newpwd", client_info, user.name);
            log_msg(log_str);
            int ret = request_newpwd(&mysql, &packet, &user, client.sockfd);
            if(ret == CORRECT){
                sprintf(log_str, "%s %s newpwd success", client_info, user.name);
            }else{
                sprintf(log_str, "%s %s newpwd error", client_info, user.name);
            }
            log_msg(log_str);

        } else if (packet.type == SEARCH) {
            sprintf(log_str, "%s %s request search", client_info, user.name);
            log_msg(log_str);
            int ret = request_search(&mysql, &packet, client.sockfd);
            if(ret == SEARCH_SUCCESS){
                sprintf(log_str, "%s %s search success", client_info, user.name);
            }else if(ret == SEARCH_FAILED){
                sprintf(log_str, "%s %s search failed", client_info, user.name);
            }else{
                sprintf(log_str, "%s %s search error", client_info, user.name);
            }
            log_msg(log_str);

        } else if (packet.type == HISTORY) {
            sprintf(log_str, "%s %s request history", client_info, user.name);
            log_msg(log_str);
            int ret = request_history(&mysql, &packet, &user, client.sockfd);
            if(ret == HISTORY_SUCCESS){
                sprintf(log_str, "%s %s history success", client_info, user.name);
            }else if(ret == HISTORY_FAILED){
                sprintf(log_str, "%s %s history failed", client_info, user.name);
            }else{
                sprintf(log_str, "%s %s history error", client_info, user.name);
            }
            log_msg(log_str);

        } else if (packet.type == QUIT) {
            sprintf(log_str, "%s %s quit", client_info, user.name);
            log_msg(log_str);
        }
    }
    sprintf(log_str, "client %s:%d disconnected", inet_ntoa(client.addr.sin_addr), ntohs(client.addr.sin_port));
    log_msg(log_str);
    mysql_close(&mysql);
    close(client.sockfd);
    free(arg);
    return NULL;
}
