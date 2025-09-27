#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<stdlib.h>
#include<fcntl.h>
#include<time.h>
#include"utils.h"

//服务器端日志记录函数
int log_msg(char* str) {
    time_t now;
    now = time(NULL);
    char* time_str = asctime(localtime(&now));
    time_str[strlen(time_str) - 1] = '\0';
    char log_str[1024];
    sprintf(log_str, "%s %s\n", time_str, str);
    int fd = open("log.txt", O_WRONLY | O_APPEND | O_CREAT, 0777);
    write(fd, log_str, strlen(log_str));
    close(fd);
    return 0;
}

//自定义输入函数
void str_input(char* str, size_t size, char* prompt) {

    while (1) {
        str[0] = '\0';
        fgets(str, sizeof(str), stdin);
        str[strcspn(str, "\n")] = 0;

        if (strlen(str) == 0 || strlen(str) > size) {
            printf("Invalid input, please try again.\n");
            continue;
        } else {
            break;
        }
    }

    if (prompt != NULL) {
        printf("%s\n", prompt);
    }

    return;
}

//数据包发送函数
int send_packet(int sockfd, int type, size_t size, void* data) {
    packet_t* p = (packet_t*)malloc(sizeof(packet_t) + size);
    if (p == NULL) {
        perror("malloc");
        return -1;
    }
    p->type = type;
    p->size = size;
    if (data == NULL) {
        p->size = 0;
        if (send(sockfd, p, sizeof(packet_t), 0) == -1) {
            perror("send");
            return -1;
        }
    } else {
        memcpy(p->data, data, size);
        if (send(sockfd, p, sizeof(packet_t) + size, 0) == -1) {
            perror("send");
            return -1;
        }
    }
    free(p);
    return 0;
}

//line
void line(){
    printf("\n-----------------------------------------------------------------------\n");
}

//提示函数
void prompt(int flag_clr, char* str, int flag_line) {
    if (flag_clr == 1) {
        system("clear");
    }
    if(flag_line == 1 || flag_line == 3){
        line();
    }
    if (str != NULL) {
        printf("\n%s\n", str);
    }
    if (flag_line == 2 || flag_line == 3) {
        line();
    }
    return;
}
