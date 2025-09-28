#include<sys/resource.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/types.h>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<signal.h>
#include<fcntl.h>
#include<stdio.h>
#include<time.h>
#include"dict.h"

void* allocated_mem[1024] = { NULL };
int mem_count = 0;

int main(int argc, char* argv[]) {
    daemonize();
    setup_signal();
    system_run(argc, argv);
    return 0;
}

//守护进程函数
void daemonize() {

    daemon(1, 0);
    int fd = open("log.txt", O_WRONLY | O_APPEND | O_CREAT, 0666);
    if (fd < 0) {
        exit(0);
    }
    dup2(fd, 1);//标准输出重定向到log.txt
    dup2(fd, 2);
    close(fd);
    return;
}

//系统运行函数
void system_run(int argc, char* argv[]) {

    if (argc != 2) {
        return;
    }

    int sockfd = server_init(argv);
    if (sockfd == -1) {
        log_msg("server init error", NULL, NULL);
        return;
    } else {
        log_msg("server init success", NULL, NULL);
    }

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

        char client_info[128] = { 0 };
        sprintf(client_info, "client %s:%d", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        log_msg(client_info, "connected", NULL);

        client_t* temp = (client_t*)malloc(sizeof(client_t));
        if (temp == NULL) {
            close(sockfd);
            send_packet(sockfd, ERROR, 0, NULL);
            continue;
        }
        reg_mem(temp);
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
            if (ret == REG_SUCCESS) {
                log_msg(client_info, "register success", NULL);
            } else if (ret == REG_FAILED) {
                log_msg(client_info, "register failed", NULL);
            } else {
                log_msg(client_info, "register error", NULL);
            }

        } else if (packet.type == LOGIN) {

            log_msg(client_info, "request login", NULL);

            int ret = request_login(&mysql, &packet, &user, client.sockfd);
            if (ret == LOGIN_SUCCESS) {
                log_msg(client_info, "login success", NULL);
            } else if (ret == LOGIN_FAILED) {
                log_msg(client_info, "login failed", NULL);
            } else {
                log_msg(client_info, "login error", NULL);
            }

        } else if (packet.type == NEWPWD) {

            log_msg(client_info, "request newpwd", NULL);

            int ret = request_newpwd(&mysql, &packet, &user, client.sockfd);
            if (ret == NEWPWD_SUCCESS) {
                log_msg(client_info, "newpwd success", NULL);
            } else {
                log_msg(client_info, "newpwd error", NULL);
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
    reg_mem(p);
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

//服务器端日志记录函数
int log_msg(char* str1, char* str2, char* str3) {
    time_t now;
    now = time(NULL);
    char* time_str = asctime(localtime(&now));
    time_str[strlen(time_str) - 1] = '\0';
    if (str2 == NULL) {
        printf("%s %s\n", time_str, str1);
    } else if (str3 == NULL) {
        printf("%s %s %s\n", time_str, str1, str2);
    } else {
        printf("%s %s %s %s\n", time_str, str1, str2, str3);
    }
    fflush(stdout);
    return 0;
}

void reg_mem(void* ptr) {
    if (mem_count < 1024) {
        allocated_mem[mem_count++] = ptr;
    }
    return;
}

//退出信号处理函数
void signal_handler(int sig) {

    log_msg("server exit", NULL, NULL);

    struct rlimit rl;
    getrlimit(RLIMIT_NOFILE, &rl);
    int max_fd = rl.rlim_max;

    //关闭所有文件描述符
    for (int fd = 3; fd < max_fd; fd++) {
        close(fd);
    }
    close(0);
    close(1);
    close(2);
    //释放所有分配的内存
    for (int i = 0; i < mem_count; i++) {
        if (allocated_mem[i]) {
            free(allocated_mem[i]);
            allocated_mem[i] = NULL;
        }
    }
    mem_count = 0;
    //退出程序
    exit(0);
}

//信号处理函数设置
void setup_signal() {
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
    return;
}