#pragma once

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define BUF_SIZE 70000
#define MAXHISTORYLENGTH 4096
#define MAXLENGTH 70000
#define MINLENGTH 1000
#define USERLENGTH 40
#define MAXNUMBEROFUSERS 20
#define NUMBEROFSTR 50
#define NUMBEROFMESSAGES 50
#define TIMELENGTH 8
#define NUM_THREADS 20
#define MIN_BUF 6

pthread_mutex_t data_mutex;

typedef struct CUserInfo {
    int online; // Если зарегетрирован - 1, иначе - 0.
    int user_id;
    char login[USERLENGTH];
    char password[USERLENGTH];
} CUserInfo;

typedef struct CClientStr {
    char status;
    char text[NUMBEROFSTR][MAXLENGTH];
    int number_of_str;
} CClientStr;

typedef struct CUser {
    int online; // Если онлайн - 1, иначе - 0.
    int socket_id; // id сокета.
    int user_id;
    int thread_id;
    char login[USERLENGTH];
    char password[USERLENGTH];
} CUser;

typedef struct CServerMood {
    int root;
    CUser users[MAXNUMBEROFUSERS];
    int thread_counter;
    pthread_t threads[NUM_THREADS];
    int number_of_users;
    char history[NUMBEROFMESSAGES][MAXLENGTH]; // хранит всё сообщение: 'r' + timestamp + login + body
    int mes_in_history_counter;
} CServerMood;

void addNewUser(int socket, int thread, char* login, char* password, CServerMood* server_mood, CUserInfo* user_info);

int isAlredyRegistreted(CServerMood* server_mood, char* login);

void sendMessageToOnlineUsers(CServerMood* server_mood, char* message, int message_size);

void generateStatusMes(int socket_id, int status);

int statusR(CClientStr* client_str, CServerMood* server_mood, int socket, CUserInfo* user_info);

int statusL(CClientStr* client_str, CServerMood* server_mood, int socket, CUserInfo* user_info);

int statusO(CClientStr* client_str, CServerMood* server_mood, int socket, CUserInfo* user_info);

int statusI(CClientStr* client_str, CServerMood* server_mood, int socket, int thread, CUserInfo* user_info);

int statusH(CClientStr* client_str, CServerMood* server_mood, int socket, CUserInfo* user_info, int number_of_mes);

int statusK(CClientStr* client_str, CServerMood* server_mood, int socket, CUserInfo* user_info, int kick_id);

void generateMMes(CServerMood* server_mood, char reason, char* login, int user_id);

int readClientString(char* string, int length, int socket, int thread, CServerMood* server_mood, CUserInfo* user_info);

