#include<stdio.h>
#include<arpa/inet.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<unistd.h>
#include<time.h>
#include<string.h>
#include"dict.h"
#include"utils.h"

int client_init(int argc, char* argv[]);
int client_register(int sockfd, user_t* user);
int client_login(int sockfd, user_t* user);
void client_function(int sockfd, packet_t* packet, user_t* user);


int main(int argc, char* argv[]) {

    int sockfd = client_init(argc, argv);
    if(sockfd == -1){
        prompt(1, "Client init error!", 3);
        return -1;
    }

    packet_t client_start;
    recv(sockfd, &client_start, sizeof(packet_t), 0);
    if(client_start.type == ERROR){
        printf("Server error, please try again later!\n");
        return -1;
    }
    recv(sockfd, &client_start, sizeof(packet_t), 0);
    if(client_start.type == ERROR){
        printf("Database init error, please try again later!\n");
        return -1;
    }

    while (1) {

        printf("+----------------------+\n");
        printf("|     1 Register       |\n");
        printf("|     2 Login          |\n");
        printf("|     3 Quit           |\n");
        printf("+----------------------+\n");
        printf("Please enter your choice: \n");

        char buf[128] = { 0 };
        str_input(buf, 127, NULL);

        packet_t packet = { 0 };
        user_t user_info = { 0 };

        if (strcmp(buf, "1") == 0) {
            if (client_register(sockfd, &user_info) == -1) {
                continue;
            }
            client_function(sockfd, &packet, &user_info);
        } else if (strcmp(buf, "2") == 0) {
            if (client_login(sockfd, &user_info) == -1) {
                continue;
            }
            client_function(sockfd, &packet, &user_info);
        } else if (strcmp(buf, "3") == 0) {
            break;
        } else {
            prompt(1, "Invalid input!", 3);
        }
    }

    prompt(1, "Client quit", 3);
    close(sockfd);
    return 0;
}

//客户端初始化函数
int client_init(int argc, char* argv[]) {

    prompt(1, NULL, 0);

    if (argc != 3) {
        printf("Usage: %s <ip> <port>\n", argv[0]);
        return -1;
    }

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket error");
        return -1;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);

    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect error");
        return -1;
    }

    return sockfd;
}

//客户端注册函数
int client_register(int sockfd, user_t* user) {

    prompt(1, "Please enter your username:", 1);
    str_input(user->name, 31, NULL);
    printf("Please enter your password:");
    str_input(user->password, 31, NULL);

    if (send_packet(sockfd, REG, sizeof(user_t), user) == -1) {
        prompt(1, "Send packet error, please try again later!", 3);
        return -1;
    }

    packet_t packet = { 0 };
    recv(sockfd, &packet, sizeof(packet_t), 0);
    if (packet.type == REG_SUCCESS) {
        prompt(1, "Register success!", 3);
    } else if (packet.type == REG_FAILED) {
        prompt(1, "Username already exists!", 3);
        return -1;
    } else if (packet.type == ERROR) {
        prompt(1, "Error!", 3);
        return -1;
    }
    return 0;
}

//客户端登录函数
int client_login(int sockfd, user_t* user) {

    prompt(1, "Please enter your username:", 1);
    str_input(user->name, 31, NULL);
    printf("Please enter your password: \n");
    str_input(user->password, 31, NULL);

    if (send_packet(sockfd, LOGIN, sizeof(user_t), user) == -1) {
        prompt(1, "Send packet error, please try again later!", 3);
        return -1;
    }

    packet_t packet = { 0 };
    recv(sockfd, &packet, sizeof(packet_t), 0);
    if (packet.type == LOGIN_SUCCESS) {
        prompt(1, "Login success!", 3);
    } else if (packet.type == LOGIN_FAILED) {
        prompt(1, "Username or password error!", 3);
        return -1;
    } else if (packet.type == ERROR) {
        prompt(1, "Error!", 3);
        return -1;
    }
    return 0;
}

//客户端功能函数
void client_function(int sockfd, packet_t* packet, user_t* user) {
    while (1) {
        printf("Welcome, %s!\n", user->name);
        printf("+---------------------------------+\n");
        printf("|     1 Change Password           |\n");
        printf("|     2 Search Word               |\n");
        printf("|     3 Open History              |\n");
        printf("|     4 Return to Main Menu       |\n");
        printf("+---------------------------------+\n");
        printf("Please enter your choice: \n");
        char buf[128] = { 0 };
        str_input(buf, 127, NULL);

        if (strcmp(buf, "1") == 0) {

            prompt(1, "Please enter your new password: ", 1);
            str_input(user->password, 31, NULL);

            if (-1 == send_packet(sockfd, NEWPWD, sizeof(user_t), user)) {
                prompt(1, "Send packet error, please try again later!", 3);
                continue;
            }

            recv(sockfd, packet, sizeof(packet_t), 0);
            if (packet->type == NEWPWD_SUCCESS) {
                prompt(1, "Change password success!", 3);
            } else if (packet->type == ERROR) {
                prompt(1, "Error!", 3);
            }

        } else if (strcmp(buf, "2") == 0) {

            prompt(1, "Please enter the word you want to search: ", 1);
            char word[128] = { 0 };
            str_input(word, 31, NULL);

            history_t history = { 0 };
            strcpy(history.user, user->name);
            time_t now = time(NULL);
            char* time_str = asctime(localtime(&now));
            time_str[strlen(time_str) - 1] = '\0';
            strcpy(history.time, time_str);
            strcpy(history.word, word);

            if (-1 == send_packet(sockfd, SEARCH, sizeof(history_t), &history)) {
                prompt(1, "Send packet error, please try again later!", 3);
                continue;
            }

            char meaning[256] = { 0 };
            recv(sockfd, packet, sizeof(packet_t), 0);
            if (packet->type == SEARCH_SUCCESS) {
                recv(sockfd, meaning, packet->size, 0);
                prompt(1, word, 1);
                prompt(0, meaning, 2);
            } else if (packet->type == SEARCH_FAILED) {
                prompt(1, "Word not found!", 3);
            } else if (packet->type == ERROR) {
                prompt(1, "Error!", 3);
            }
        } else if (strcmp(buf, "3") == 0) {

            if (-1 == send_packet(sockfd, HISTORY, 0, NULL)) {
                prompt(1, "Send packet error, please try again later!", 3);
                continue;
            }

            recv(sockfd, packet, sizeof(packet_t), 0);

            if (packet->type == HISTORY_SUCCESS) {
                prompt(1, NULL, 1);
                printf("+---------------------------------------------------------------+\n");
                printf("|  Time                      |  Word\t\t\t\t|\n");
                printf("|----------------------------|----------------------------------|\n");
                while (1) {
                    history_t history = { 0 };
                    recv(sockfd, packet, sizeof(packet_t), 0);
                    if (packet->type == HISTORY_TRANS_FINISHED) {
                        printf("+---------------------------------------------------------------+\n");
                        prompt(0, "Retrieve finish!", 2);
                        break;
                    }
                    recv(sockfd, &history, packet->size, 0);
                    printf("|  %s  |  %-32s|\n", history.time, history.word);
                }
            } else if (packet->type == HISTORY_FAILED) {
                prompt(1, "History not found!", 3);
            } else if (packet->type == ERROR) {
                prompt(1, "Error!", 3);
            }

        } else if (strcmp(buf, "4") == 0) {
            prompt(1, NULL, 0);
            send_packet(sockfd, QUIT, 0, NULL);
            break;
        } else {
            prompt(1, "Invalid input", 3);
        }
    }
    return;
}

