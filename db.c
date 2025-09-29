#include<string.h>
#include<stdio.h>
#include<time.h>
#include"dict.h"

//数据库初始化
int db_init(MYSQL* mysql) {

    if (mysql_init(mysql) == NULL) {
        log_msg("database init error", "mysql_init", mysql_error(mysql));
        return -1;
    }

    if (mysql_real_connect(mysql, "localhost", "kerwin", "231925", "dict", 0, NULL, 0) == NULL) {
        log_msg("database init error", "mysql_real_connect", mysql_error(mysql));
        return -1;
    }

    if (mysql_set_character_set(mysql, "utf8") != 0) {
        log_msg("database init error", "mysql_set_character_set", mysql_error(mysql));
        return -1;
    }

    log_msg("database init success", NULL, NULL);

    return 0;
}

//用户注册
int request_register(MYSQL* mysql, packet_t* packet, user_t* user, int sockfd) {

    recv(sockfd, user, packet->size, 0);

    char sql[128];
    sprintf(sql, "SELECT * FROM user WHERE 用户名 = '%s'", user->name);
    if (mysql_query(mysql, sql)) {
        log_msg("requset_register error", "mysql_query", mysql_error(mysql));
        send_packet(sockfd, ERROR, 0, NULL);
        return ERROR;
    }

    MYSQL_RES* res = mysql_store_result(mysql);
    if (res == NULL) {
        log_msg("requset_register error", "mysql_store_result", mysql_error(mysql));
        send_packet(sockfd, ERROR, 0, NULL);
        return ERROR;
    }

    int rows = mysql_num_rows(res);
    if (rows == 1) {
        send_packet(sockfd, FAILED, 0, NULL);
        return FAILED;
    } else {
        memset(sql, 0, 128);
        sprintf(sql, "INSERT INTO user VALUES ('%s', '%s')", user->name, user->password);
        if (mysql_query(mysql, sql)) {
            log_msg("requset_register error", "mysql_query", mysql_error(mysql));
            send_packet(sockfd, ERROR, 0, NULL);
            return ERROR;
        }
        send_packet(sockfd, SUCCESS, 0, NULL);
    }
    return SUCCESS;
}

//用户登录
int request_login(MYSQL* mysql, packet_t* packet, user_t* user, int sockfd) {

    recv(sockfd, user, packet->size, 0);
    char sql[128];
    sprintf(sql, "SELECT * FROM user WHERE 用户名 = '%s' AND 密码 = '%s'", user->name, user->password);
    if (mysql_query(mysql, sql)) {
        log_msg("request_login error", "mysql_query", mysql_error(mysql));
        send_packet(sockfd, ERROR, 0, NULL);
        return ERROR;
    }

    MYSQL_RES* res = mysql_store_result(mysql);
    if (res == NULL) {
        log_msg("request_login error", "mysql_store_result error", mysql_error(mysql));
        send_packet(sockfd, ERROR, 0, NULL);
        return ERROR;
    }

    int rows = mysql_num_rows(res);
    if (rows == 0) {
        send_packet(sockfd, FAILED, 0, NULL);
        return FAILED;
    } else {
        send_packet(sockfd, SUCCESS, 0, NULL);
    }
    return SUCCESS;
}

//用户修改密码
int request_newpwd(MYSQL* mysql, packet_t* packet, user_t* user, int sockfd) {

    recv(sockfd, user, packet->size, 0);
    char sql[128];
    sprintf(sql, "UPDATE user SET 密码 = '%s' WHERE 用户名 = '%s'", user->password, user->name);
    if (mysql_query(mysql, sql)) {
        log_msg("request_newpwd error", "mysql_query error", mysql_error(mysql));
        send_packet(sockfd, ERROR, 0, NULL);
        return ERROR;
    }
    send_packet(sockfd, SUCCESS, 0, NULL);
    return SUCCESS;
}

//查询单词
int request_search(MYSQL* mysql, packet_t* packet, int sockfd) {
    char word[32] = { 0 };
    char meaning[256] = { 0 };
    history_t history = { 0 };
    recv(sockfd, &history, packet->size, 0);
    strcpy(word, history.word);

    db_history_add(mysql, history);

    char sql[128];
    sprintf(sql, "SELECT * FROM word WHERE 单词 = '%s'", word);
    if (mysql_query(mysql, sql)) {
        log_msg("request_search error", "mysql_query error", mysql_error(mysql));
        send_packet(sockfd, ERROR, 0, NULL);
        return ERROR;
    }

    MYSQL_RES* res = mysql_store_result(mysql);
    if (res == NULL) {
        log_msg("request_search error", "mysql_store_result error", mysql_error(mysql));
        send_packet(sockfd, ERROR, 0, NULL);
        return ERROR;
    }

    int rows = mysql_num_rows(res);
    if (rows == 0) {
        send_packet(sockfd, FAILED, 0, NULL);
        return FAILED;
    } else {
        MYSQL_ROW row = mysql_fetch_row(res);
        strcpy(meaning, row[1]);
        send_packet(sockfd, SUCCESS, strlen(meaning) + 1, meaning);
    }
    return SUCCESS;
}

//添加历史记录
int db_history_add(MYSQL* mysql, history_t history) {
    char sql[256];
    sprintf(sql, "INSERT INTO history VALUES ('%s', '%s', '%s')", history.user, history.time, history.word);
    if (mysql_query(mysql, sql)) {
        log_msg("db_history_add error", "mysql_query error", mysql_error(mysql));
        return -1;
    }
    return 0;
}

//查询历史记录
int request_history(MYSQL* mysql, packet_t* packet, user_t* user, int sockfd) {

    char sql[128];
    sprintf(sql, "SELECT * FROM history WHERE 用户名 = '%s'", user->name);
    if (mysql_query(mysql, sql)) {
        log_msg("request_history error", "mysql_query error", mysql_error(mysql));
        send_packet(sockfd, ERROR, 0, NULL);
        return ERROR;
    }

    MYSQL_RES* res = mysql_store_result(mysql);
    if (res == NULL) {
        log_msg("request_history error", "mysql_store_result error", mysql_error(mysql));
        send_packet(sockfd, ERROR, 0, NULL);
        return ERROR;
    }

    int rows = mysql_num_rows(res);
    if (rows == 0) {
        send_packet(sockfd, FAILED, 0, NULL);
        return FAILED;
    } else if (rows > 0) {
        send_packet(sockfd, SUCCESS, 0, NULL);
        MYSQL_ROW row;
        for (int i = 1; i <= rows; i++) {
            row = mysql_fetch_row(res);
            history_t history = { 0 };
            strcpy(history.user, row[0]);
            strcpy(history.time, row[1]);
            strcpy(history.word, row[2]);
            send_packet(sockfd, SUCCESS, sizeof(history_t), &history);
        }
        send_packet(sockfd, HISTORY_TRANS_FINISHED, 0, NULL);
    }
    return SUCCESS;
}
