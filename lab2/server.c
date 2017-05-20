#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "receive_mes.h"

#define SOCKET 12340

void server_initialization(CServerMood* server_mood) {
    server_mood->root = -1;
    server_mood->number_of_users = 0;
    server_mood->mes_in_history_counter = 0;
    server_mood->thread_counter = 0;
}

int addRoot(CServerMood* server_mood, char* password){
    int i;
    // Check password for correctness:
    if (strlen(password) < 2 || strlen(password) > 31) {
        return 1;
    }
    for (i = 0; i < strlen(password); ++i) {
        if (password[i] < ' ' || password[i] > '~') {
            return 1;
        }
    }
    server_mood->users[0].user_id = 0;
    server_mood->number_of_users += 1;
    server_mood->users[0].socket_id = -1;
    server_mood->users[0].online = 0;
    strcpy(server_mood->users[0].login, "root");
    strcpy(server_mood->users[0].password, password);
    return 0;
}


int main() {
    int sockid = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    CSomeArgs args;
    CServerMood server_mood;
    server_initialization(&server_mood);

    args.current = &server_mood;
    if (sockid == -1) {
        fprintf(stderr, "Socket opening failed\n");
        return 1;
    }
    
    struct sockaddr_in port;
    port.sin_family = AF_INET;
    port.sin_port = htons(SOCKET);
    port.sin_addr.s_addr = htonl(INADDR_ANY);
    int status = bind(sockid, (struct sockaddr *) &port, sizeof(port));

    if (status == -1) {
        close(sockid);
        fprintf(stderr, "Socket binding failed\n");
        return 2;
    }

    printf("Введите пароль для root\n");
    char* line;
    size_t len = 0;
    ssize_t read;
    read = getline(&line, &len, stdin);
    line[read - 1] = 0;
    while(addRoot(&server_mood, line)) {
        read = getline(&line, &len, stdin);
    }

    status = listen(sockid, 1);

    if (status == -1) {
        fprintf(stderr, "Socket listening failed\n");
        return 3;
    }

    printf("Listening to port %d...\n", ntohs(port.sin_port));

    if (pthread_mutex_init(&data_mutex, NULL) != 0) {
        printf("mutex init failed\n");
        return 1;
    }
    
    for(;;) {
        struct sockaddr client_addr;
        int addr_len = sizeof(client_addr);
        int client_sock = accept(sockid, &client_addr, &addr_len);
    
        if (client_sock == -1) {
            fprintf(stderr, "Socket accepting failed\n");
            return 4;
        }

        pthread_mutex_lock(&data_mutex);
        printf("Connection accepted %d...\n", ntohs(port.sin_port));
        printf("Connected to client %d\n",client_sock);
        
        int* new_sock;
        new_sock = malloc(1);
        *new_sock = client_sock;
        args.id = new_sock;
        args.thread_id = server_mood.thread_counter;

        status = pthread_create(&server_mood.threads[server_mood.thread_counter++] , NULL ,  processConnection, (void*) &args);
        if(status == -1) {
            fprintf(stderr, "Could not create thread\n");
            return 5;
        }
        pthread_mutex_unlock(&data_mutex);
        
        printf("Handler assigned %d\n", sockid);
    }
    
    close(sockid);

    return 0;
}
