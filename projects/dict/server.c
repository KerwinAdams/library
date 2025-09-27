#include<sys/resource.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/types.h>
#include<pthread.h>
#include<signal.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<fcntl.h>
#include<stdio.h>
#include<time.h>
#include"dict.h"

int log_fd = -1;

void sig_handler(int sig) {

    struct rlimit rlim;
    int max_fd;
    int fd;

    if (getrlimit(RLIMIT_NOFILE, &rlim) == -1) {
        max_fd = 1024;
    }else{
        max_fd = rlim.rlim_max;
    }

    for(fd = 3; fd < max_fd; fd++){
        close(fd);
    }

    exit(0);
}

int main(int argc, char* argv[]) {
    printf("%d\n",getpid());
    //daemon(0, 0);
    system_run(argc, argv);
    return 0;
}

//系统运行函数
void system_run(int argc, char* argv[]) {

    signal(SIGINT, sig_handler);

    if (argc != 2) {
        return;
    }

    int sockfd = server_init(argv);
    if (sockfd == -1) {
        log_msg("server init error", NULL, NULL);
        return;
    }

    log_msg("server start", NULL, NULL);

    server_loop(sockfd);
    return;
}

//日志初始化函数
void log_init() {
    log_fd = open("/temp/dcit_server_log.txt", O_WRONLY | O_APPEND | O_CREAT, 0666);
    if (log_fd == -1) {
        printf("open log file error\n");
        exit(1);
    }
    return;
}

//服务器端日志记录函数
int log_msg(char* str1, char* str2, char* str3) {
    time_t now;
    now = time(NULL);
    char* time_str = asctime(localtime(&now));
    time_str[strlen(time_str) - 1] = '\0';
    char log_str[1024];
    if (str2 == NULL) {
        sprintf(log_str, "%s %s\n", time_str, str1);
    } else if (str3 == NULL) {
        sprintf(log_str, "%s %s %s\n", time_str, str1, str2);
    } else {
        sprintf(log_str, "%s %s %s %s\n", time_str, str1, str2, str3);
    }
    write(log_fd, log_str, strlen(log_str));
    return 0;
}

//服务器初始化函数
int server_init(char* argv[]) {

    log_init();

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

        char client_info[128] = { 0 };
        sprintf(client_info, "client %s:%d", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        log_msg(client_info, "connected", NULL);

        client_t* temp = (client_t*)malloc(sizeof(client_t));
        temp->sockfd = sockfd;
        temp->addr = client_addr;

        pthread_t tid;
        if (pthread_create(&tid, NULL, client_thread, temp) != 0) {
            close(sockfd);
            free(temp);
            send_packet(sockfd, ERROR, 0, NULL);
            continue;
        }

        if (pthread_detach(tid) != 0) {
            close(sockfd);
            free(temp);
            send_packet(sockfd, ERROR, 0, NULL);
            continue;
        }

        send_packet(sockfd, CORRECT, 0, NULL);
    }
}

//客户端线程函数
void* client_thread(void* arg) {

    client_t client = *(client_t*)arg;

    char client_info[128] = { 0 };
    sprintf(client_info, "client %s:%d", inet_ntoa(client.addr.sin_addr), ntohs(client.addr.sin_port));

    MYSQL mysql;
    if (db_init(&mysql) == -1) {
        log_msg(client_info, "database init error", NULL);
        send_packet(client.sockfd, ERROR, 0, NULL);
        goto cleanup;
    }

    log_msg(client_info, "database init success", NULL);
    send_packet(client.sockfd, CORRECT, 0, NULL);

    packet_t packet;
    user_t user;

    while (1) {
        memset(&packet, 0, sizeof(packet_t));
        if (recv(client.sockfd, &packet, sizeof(packet_t), 0) == 0) {
            break;
        }

        if (packet.type == REG) {

            log_msg(client_info, "request register", NULL);

            int ret = request_register(&mysql, &packet, &user, client.sockfd);
            switch (ret) {
            case REG_SUCCESS:
                log_msg(client_info, "register success", NULL);
                break;
            case REG_FAILED:
                log_msg(client_info, "register failed", NULL);
                break;
            default:
                log_msg(client_info, "register error", NULL);
                break;
            }

        } else if (packet.type == LOGIN) {

            log_msg(client_info, "request login", NULL);
            int ret = request_login(&mysql, &packet, &user, client.sockfd);
            switch (ret) {
            case LOGIN_SUCCESS:
                log_msg(client_info, "login success", NULL);
                break;
            case LOGIN_FAILED:
                log_msg(client_info, "log_msgin failed", NULL);
                break;
            default:
                log_msg(client_info, "log_msgin error", NULL);
                break;
            }

        } else if (packet.type == NEWPWD) {

            log_msg(client_info, "request newpwd", NULL);
            int ret = request_newpwd(&mysql, &packet, &user, client.sockfd);
            switch (ret) {
            case CORRECT:
                log_msg(client_info, "newpwd success", NULL);
                break;
            default:
                log_msg(client_info, "newpwd error", NULL);
                break;
            }

        } else if (packet.type == SEARCH) {

            log_msg(client_info, user.name, "request search");
            int ret = request_search(&mysql, &packet, client.sockfd);
            if (ret == SEARCH_SUCCESS) {
                log_msg(client_info, user.name, "search success");
            } else if (ret == SEARCH_FAILED) {
                log_msg(client_info, user.name, "search failed");
            } else {
                log_msg(client_info, user.name, "search error");
            }

        } else if (packet.type == HISTORY) {

            log_msg(client_info, user.name, "request history");
            int ret = request_history(&mysql, &packet, &user, client.sockfd);
            if (ret == HISTORY_SUCCESS) {
                log_msg(client_info, user.name, "history success");
            } else if (ret == HISTORY_FAILED) {
                log_msg(client_info, user.name, "history failed");
            } else {
                log_msg(client_info, user.name, "history error");
            }

        } else if (packet.type == QUIT) {
            log_msg(client_info, user.name, "quit");
        }
    }
    log_msg(client_info, "disconnected", NULL);
cleanup:
    mysql_close(&mysql);
    close(client.sockfd);
    free(arg);
    return NULL;
}

//数据包发送函数
int send_packet(int sockfd, int type, size_t size, void* data) {
    packet_t* p = (packet_t*)malloc(sizeof(packet_t) + size);
    if (p == NULL) {
        return -1;
    }
    p->type = type;
    p->size = size;
    if (data == NULL) {
        p->size = 0;
        if (send(sockfd, p, sizeof(packet_t), 0) == -1) {
            return -1;
        }
    } else {
        memcpy(p->data, data, size);
        if (send(sockfd, p, sizeof(packet_t) + size, 0) == -1) {
            return -1;
        }
    }
    free(p);
    return 0;
}