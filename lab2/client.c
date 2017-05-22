#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "useful.h"


//#define MAXLENGTH 65535
#define BUF_SIZE 70000
#define TIME 50
#define MAXLENGTH 70000
#define MAXHISTORY 50
#define NUMBEROFSTR 55
#define SOCKET 12340
#define MIN_BUF 10

typedef struct CArgs {
    int* id;
    pthread_t thread;
} CArgs;

typedef struct CServerStr {
    size_t sec[TIME];
    size_t mic[TIME];
    char status;
    char text[NUMBEROFSTR][MAXLENGTH];
    int number_of_str;
} CServerStr;

int generateKickMessage(char* message, int number_of_str, char* text[]) {
    //printf("generateKickMessage\n");
    int i;
    int length = 0;
    int total_lenght = 0;
    int user_id;
    size_t len;
    ssize_t read;
    char id[MIN_BUF];
    //printf("1\n");
    while (scanf("%d", &user_id) == -1) {
        printf("Invalid input, please try again\n");
    }
    //printf("user_id: %d\n", user_id);
    //printf("2\n");

    intToBigEndian(id, user_id);
    //printf("3\n");

    memcpy(text[1], id, 4);
    /*printf("text[1]\n");
    for (i = 0; i < 4; i++) {
        printf("%X ", text[1][i]);
    }*/
    text[1][5] = 0;
    number_of_str += 1;

    while ((read = getline(&text[number_of_str], &len, stdin)) != -1) {
        while (text[number_of_str][0] == '\n') {
            getline(&text[number_of_str], &len, stdin);
        }
        number_of_str++;
    }

    size_t message_size = clientConstructingMessage(message, text, 4);

    /*printf("\ngenerateKickMessage:\n");
    for ( i = 0; i < message_size; i++) {
        printf("%X ", message[i]);
    }*/
    return message_size;
}

int generateHistoryMessage(char* message) {
    int length = 0;
    int number_of_mes;
    while (scanf("%d", &number_of_mes) == -1) {
        printf("Invalid input, please try again.\n");
    }
    number_of_mes = min(number_of_mes, MAXHISTORY);
    memcpy(message, "h", 1);
    length += 1;
    intToBigEndian(message + length, 8);
    length += 4;
    intToBigEndian(message + length, 4);
    length += 4;
    intToBigEndian(message + length, number_of_mes);
    length += 4;
    message[length] = '\0';
    return length;
}

void* sendMessage(void* socket_desc) {
    int socket_id = *(int*)socket_desc;
    int j;
    int i;
    for (;;) {
        //sleep(10);
        printf("\nEnter text to send\n");
        char message[MAXLENGTH]; 

        size_t len = 0;
        ssize_t read;
        size_t messageSize;
        char *text[MAXLENGTH];

        int number_of_str = 0;

        // Считывает следующую пустую строку, потому что делаем break
        while (number_of_str == 0) {
            while ((read = getline(&text[number_of_str], &len, stdin)) != -1) {
                if (text[0][0] == '\n') {
                    getline(&text[number_of_str], &len, stdin);
                }
                if (strlen(text[number_of_str]) > 65535) {
                    printf("Invalid message, please try again\n");
                    number_of_str -= 1;
                }
                if (strlen(text[0]) != 2) {
                    printf("Invalid status, please try again\n");
                    number_of_str -= 1;
                } else {
                    if (text[0][0] == 'h' || text[0][0] == 'k') {
                        number_of_str += 1;
                        break;
                    }
                }
                number_of_str += 1;
            }
            if (number_of_str == 0) {
                printf("Invalid message, please try again\n");
            }
        }

        if (text[0][0] == 'h') {
            messageSize = generateHistoryMessage(message);
        } else {
            if (text[0][0] == 'k') {
                messageSize = generateKickMessage(message, number_of_str, text);
            } else {
                messageSize = clientConstructingMessage(message, text, number_of_str);
            }
        }

        /*printf("\nsendClientMessage:\n");
        for (i = 0; i < messageSize; i++) {
            printf("%X ", message[i]);
        }

        printf("\n"); */

        int count = send(socket_id, message, messageSize, 0);
        if (count == -1) {
            fprintf(stderr, "send() failed\n");
            free(socket_desc);
            return;
        }
    }
    free(socket_desc);
    return;
}

void readServerStrH(int length, char* string, CServerStr* server_str) {
    //printf("readServerStrH\n");

    int k = 0;
    int len = 0;
    int counter = 1;
    int current_number = 0;
    int j = 0;
    int total_lenght = 0;
    /*printf("Reading from server:\n");
    for (j = 0; j < length; j++) {
        printf("%X ", string[j]);
    }
    printf("\n------------------------\n");*/
    server_str->status = string[0];

    total_lenght = getTotalLength(string);
    counter += 4;
    while (counter != total_lenght + 5) {
        counter += 4;
        //server_str->sec[k] = (string[counter + 0] << 24) | (string[counter + 1] << 16) | (string[counter + 2] << 8) | string[counter + 3];
        server_str->sec[k] = getLength(string + counter);
        k++;
        counter += 4;
        //server_str->mic[k] = (string[counter + 0] << 24) | (string[counter + 1] << 16) | (string[counter + 2] << 8) | string[counter + 3];
        server_str->mic[k] = getLength(string + counter);
        k++;
        counter += 4;
        current_number += 1;

        len = getStringLenghtFromMessage(string, current_number);
        counter += 4;
        memcpy(server_str->text[server_str->number_of_str], getStringFromMessage(string, current_number), len);
        server_str->text[server_str->number_of_str][len] = 0;
        counter += len;
        server_str->number_of_str += 1;
        current_number += 1;

        len = getStringLenghtFromMessage(string, current_number);
        counter += 4;
        memcpy(server_str->text[server_str->number_of_str], getStringFromMessage(string, current_number), len);
        server_str->text[server_str->number_of_str][len] = 0;
        counter += len;
        server_str->number_of_str += 1;
        current_number += 1;
    }
}

void readServerStr(int length, char* string, CServerStr* server_str) {
    int j = 0;
    int counter = 0;
    int total_lenght = 0;
    /*printf("Reading from server:\n");
    for (j = 0; j < length; j++) {
        printf("%X ", string[j]);
    }
    printf("\n------------------------\n");*/
    server_str->status = string[0];
    counter += 1;

    total_lenght = getTotalLength(string);
    //printf("%d\n", total_lenght);
    counter += 4;

    int flagRM = 0;
    if (string[0] == 'r' || string[0] == 'm') {
        counter += 4;
        flagRM = 1;
    }

    if (flagRM) {
        //server_str->sec[k] = (string[counter + 0] << 24) | (string[counter + 1] << 16) | (string[counter + 2] << 8) | string[counter + 3];
        server_str->sec[0] = getLength(string + counter);
        counter += 4;
        //server_str->mic[k] = (string[counter + 0] << 24) | (string[counter + 1] << 16) | (string[counter + 2] << 8) | string[counter + 3];
        server_str->mic[0] = getLength(string + counter);
        counter += 4;
    }

    // Форматирование строки
    while (counter != total_lenght + 5) {
        int len = getStringLenghtFromMessage(string, server_str->number_of_str + flagRM);
        counter += 4;
        memcpy(server_str->text[server_str->number_of_str], getStringFromMessage(string, server_str->number_of_str + flagRM), len);
        server_str->text[server_str->number_of_str][len] = 0;
        counter += len;
        server_str->number_of_str += 1;
    }
}

void writeL(CServerStr* server_str) {
    int i;
    printf("List of online users:\n");
    for (i = 0; i < server_str->number_of_str; i += 2) {
        printf("user_id: %s login: %s\n", server_str->text[i], server_str->text[i + 1]);
    }
}

void writeR(CServerStr* server_str) {
    int i;
    printf("Message from server: \n");
    printTime(server_str->sec[0]);
    //printf("time: sec %ld mic %ld\n", server_str->sec[0], server_str->mic[0]);
    printf("from user: %s\n", server_str->text[0]);
    printf("message: %s\n", server_str->text[1]);
    printf("\n");
}

void writeH(CServerStr* server_str) {
    int i = 0;
    printf("History:\n");
    for (i = 0; i < server_str->number_of_str; i += 2) {
        printTime(server_str->sec[0]);
        //printf("time: sec %ld mic %ld\n", server_str->sec[i], server_str->mic[i + 1]);
        printf("from user: %s\n", server_str->text[i]);
        printf("message: %s\n", server_str->text[i + 1]);
        printf("\n");
    }  
}

void writeK(CServerStr* server_str) {
    printf("You are kicked with reason: %s\n", server_str->text[0]);
}

void writeM(CServerStr* server_str) {
    int i;
    printTime(server_str->sec[0]);
    //printf("time: sec %ld mic %ld\n", server_str->sec[0], server_str->mic[0]);
    printf("%s\n", server_str->text[0]);
    printf("\n");
}

void writeS(size_t status) {
    if (status == 0) {
        printf("OK\n");
    } else if (status == 3) {
        printf("ошибка аутентификации\n");
    } else if (status == 2) {
        printf("Незалогиненный пользователь\n");
    } else if (status == 6) {
        printf("Invalid message\n");
    } else if (status == 5) {
        printf("Ошибка доступа\n");
    } else if (status == 4) {
        printf("Oшибка регистрации\n");
    } else if (status == 1) {
        printf("Cообщение неизвестного типа\n");
    }
}

void* recvMessage(void* args) {
    int i;
    CArgs *arg = (CArgs*) args;

    int* socket_desc = arg->id;

    pthread_t thread = arg->thread;
    int socket_id = *(int*)socket_desc;
    char buffer[BUF_SIZE];
    for (;;) {
        int count = recv(socket_id, buffer, 5, 0); 
        int total_lenght = getTotalLength(buffer);
        count = recv(socket_id, buffer + 5, total_lenght, 0);
        CServerStr server_str;
        server_str.number_of_str = 0;

        if (count == -1) {
            fprintf(stderr, "recv() failed\n");
            close(socket_id);
            free(socket_desc);
            return;
        }

        //count -= 1; 

        //printf("String from server: %s \n", buffer);
        if (buffer[0] == 's') {
            //int status = (buffer[9] << 24) | (buffer[9 + 1] << 16) | (buffer[9 + 2] << 8) | buffer[9 + 3];
            int status = getLength(buffer + 9);
            writeS(status);
        }
        if (buffer[0] == 'r') {
            readServerStr(count, buffer, &server_str);
            writeR(&server_str);
        }
        if (buffer[0] == 'l') {
            readServerStr(count, buffer, &server_str);
            writeL(&server_str);
        }
        if (buffer[0] == 'h') {
            readServerStrH(count, buffer, &server_str);
            writeH(&server_str);
        }
        if (buffer[0] == 'm') {
            readServerStr(count, buffer, &server_str);
            writeM(&server_str);
        }
        if (buffer[0] == 'k') {
            readServerStr(count, buffer, &server_str);
            writeK(&server_str);
            close(socket_id);
            free(socket_desc);
            exit(EXIT_SUCCESS);
            //int temp = pthread_cancel(thread);
            /*if (temp != 0) {
                fprintf(stderr, "pthread_cancel() failed\n");
            }*/
            return;
        }
        /*printf("free2\n");
        for (i = 0; i < server_str.number_of_str; i++) {
            free(server_str.text[i]);
        }*/
    }
    free(socket_desc);
    return;
}

void processConnection(int socket_id) {
    CArgs args;
    pthread_t thread;
    pthread_t thread1;
    int* new_sock;
    new_sock = malloc(1);
    *new_sock = socket_id;
    args.id = new_sock;
    args.thread = thread1;

    printf("Возможные команды: e (= exit), r (= regular), i (= login), o (= logout), h (= history), l (= list), k (= kick)\n");

    int status = pthread_create(&thread, NULL, sendMessage, (void*) new_sock);
    if (status == -1) {
        fprintf(stderr, "Could not create thread\n");
        return;
    }
    int status1 = pthread_create(&thread1, NULL, recvMessage, (void*) &args);
    if (status1 == -1) {
        fprintf(stderr, "Could not create thread\n");
        return;
    }
    for (;;) {

    }
}

int main() {
    int socket_id = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket_id == -1) {
        fprintf(stderr, "Socket opening failed\n");
        return 1;
    }
    
    struct sockaddr_in port;
    port.sin_family = AF_INET;
    port.sin_port = htons(SOCKET);
    port.sin_addr.s_addr = htonl(INADDR_ANY);
    int status = connect(socket_id, (struct sockaddr *) &port, sizeof(port));

    if (status == -1) {
        close(socket_id);
        fprintf(stderr, "Socket connecting failed\n");
        return 2;
    }

    printf("Connected to server %d\n", socket_id);

    processConnection(socket_id);
    
    close(socket_id);

    return 0;
}
