#include "format_mes.h"
#include "useful.h"

// Добавлем нового пользователя.
void addNewUser(int socket, int thread, char* login, char* password, CServerMood* server_mood, CUserInfo* user_info) {
    int i;
    int k = server_mood->number_of_users;
    server_mood->number_of_users += 1;
    //printf("number_of_users = %d\n", server_mood->number_of_users, sizeof(server_mood->users));
    server_mood->users[k].online = 1;
    server_mood->users[k].thread_id = thread;
    server_mood->users[k].socket_id = socket;
    server_mood->users[k].user_id = k;
    strcpy(server_mood->users[k].login, login);

    strcpy(server_mood->users[k].password, password);

    strcpy(user_info->login, login);

    user_info->user_id = k;
    strcpy(user_info->password, password);
    return;
}

// Возращаем позицию + 1
int isAlredyRegistreted(CServerMood* server_mood, char* login) {
    int i;
    for (i = 0; i < server_mood->number_of_users; i++){
        if (!strcmp(login, server_mood->users[i].login)){
            return i + 1;
        }
    }
    return 0;
}

void sendMessageToOnlineUsers(CServerMood* server_mood, char* message, int message_size) {
    pthread_mutex_lock(&data_mutex);
    int i;
    for (i = 0; i < message_size; i++){

    }
    for (i = 0; i < server_mood->number_of_users; ++i) {
        if (server_mood->users[i].online) {
            ssize_t count = send(server_mood->users[i].socket_id, message, message_size, 0);
            //printf("%d %d\n", count, message_size);
            if (count != message_size) {
                fprintf(stderr, "Can't send message %c to %s\n", message[0], server_mood->users[i].login);
            }
        }
    }
    pthread_mutex_unlock(&data_mutex);
    return;
}

void generateStatusMes(int socket_id, int status) {
    char mes[100];
    mes[0] = 's';
    intToBigEndian(mes + 1, 8);
    intToBigEndian(mes + 5, 4);
    intToBigEndian(mes + 9, status);
    send(socket_id, mes, 13, 0);
}

int statusR(CClientStr* client_str, CServerMood* server_mood, int socket, CUserInfo* user_info) {
    int i;
    if (client_str->number_of_str != 1) {
        return 6;
    } 
    //printf("statusR\n");
    char text[NUMBEROFSTR][MINLENGTH];
    char timeStr[TIMELENGTH];
    getTime(timeStr);
    strcpy(text[0], "r");
    memcpy(text[1], timeStr, 8);
    text[1][9] = 0;
    strcpy(text[2], user_info->login);;
    strcpy(text[3], client_str->text[0]);
    char message[MAXLENGTH];
    size_t message_size = serverConstructingMessage(message, text, 4);
    
    /*printf("\nstatusRmessage:\n");
    for ( i = 0; i < message_size; i++){
        printf("%X ", message[i]);
    }
    printf("server_mood->mes_in_history_counter: %d,  message_size: %d\n", server_mood->mes_in_history_counter, message_size);
    */

    // Добавляем к истории.
    memcpy(server_mood->history[server_mood->mes_in_history_counter++], message, message_size);
    sendMessageToOnlineUsers(server_mood, message, message_size);
    return 0;
}

// Считаем строки с единицы
int statusL(CClientStr* client_str, CServerMood* server_mood, int socket, CUserInfo* user_info) {
    //printf("statusL\n");
    char text[NUMBEROFSTR][MINLENGTH];
    if (client_str->number_of_str != 0) {
        return 6;
    }
    int j = 0;
    strcpy(text[j], "l");
    text[j][1] = 0;
    j++;
    int i;
    int k;

    pthread_mutex_lock(&data_mutex);
    for (i = 0; i < server_mood->number_of_users; ++i){
        if (server_mood->users[i].online) {
            IntToStr(text[j], server_mood->users[i].user_id);
            j++;
            memcpy(text[j], server_mood->users[i].login, strlen(server_mood->users[i].login));
            text[j][strlen(server_mood->users[i].login)] = '\0';
            j++;
        }
    }
    pthread_mutex_unlock(&data_mutex);
    char message[MAXLENGTH];
    size_t message_size = serverConstructingMessage(message, text, j);
    /*for(j = 0; j < message_size; j++) {
        printf("%X ", message[j]);
    }
    printf("\n");*/
    send(socket, message, message_size, 0);
}

int statusO(CClientStr* client_str, CServerMood* server_mood, int socket, CUserInfo* user_info) {
    int j;
    if (client_str->number_of_str != 0) {
        return 6;
    }
    int i;
    pthread_mutex_lock(&data_mutex);
    for (i = 0; i < server_mood->number_of_users; i++) {
        if (user_info->user_id == server_mood->users[i].user_id) {
            server_mood->users[i].online = 0;
        }
    }
    pthread_mutex_unlock(&data_mutex);
    generateStatusMes(socket, 0);
    return 0;
}

int statusI(CClientStr* client_str, CServerMood* server_mood, int socket, int thread, CUserInfo* user_info) {

    int j;
    int i;
    if (client_str->number_of_str == 2) {
        //Проверяем логин и пароль на корректность.
        for (j = 0; j < 2; ++j) {
            if (strlen(client_str->text[j]) < 2 || strlen(client_str->text[j]) > 31) {
                return 6;
            }
            for (i = 0; i < strlen(client_str->text[j]); ++i) {
                if (client_str->text[j][i] < ' ' || client_str->text[j][i] > '~') {
                    return 6;
                }
            }
        }
    } else {
       return 6;
    }
    pthread_mutex_lock(&data_mutex);
    // Если пользователь с таким же login уже зарегистрирован.
    int temp = isAlredyRegistreted(server_mood, client_str->text[0]);
    if (temp){
        temp--;
        if (!strcmp(client_str->text[1], server_mood->users[temp].password) && !server_mood->users[temp].online) {
            server_mood->users[temp].online = 1;
            server_mood->users[temp].thread_id = thread;
            server_mood->users[temp].socket_id = socket;
            user_info->online = 1;
            strcpy(user_info->login, server_mood->users[temp].login);
            user_info->user_id = server_mood->users[temp].user_id;
            strcpy(user_info->password, server_mood->users[temp].password);
        } else {
            pthread_mutex_unlock(&data_mutex);
            return 3;
        }
    } else {
        // Добавляем нового пользователя.
        addNewUser(socket, thread, client_str->text[0], client_str->text[1], server_mood, user_info);
    }
    pthread_mutex_unlock(&data_mutex);
    return 0;
}

int statusH(CClientStr* client_str, CServerMood* server_mood, int socket, CUserInfo* user_info, int number_of_mes) {
    //printf("statusH\n");
    if (number_of_mes > server_mood->mes_in_history_counter ||
        client_str->number_of_str != 1){
        return 6;
    }
    char* info;
    char text[NUMBEROFSTR][MINLENGTH];
    int length = 0;
    size_t total_length = 0;
    strcpy(text[0], "h");
    length += 1;
    pthread_mutex_lock(&data_mutex);
    int i;
    int j;
    int k;
    length += 4;

    for (i = 0; i < number_of_mes; i++) {
        total_length += getTotalLength(server_mood->history[i]);

        for(j = 0; j < 3; j++){
            info = getStringFromMessage(server_mood->history[i], j);
            if(j != 0) {

                strcpy(text[i * 3 + j + 1], info);

            } else {
                for (k = 0; k < 8; k++){
    
                    text[i * 3 + j + 1][k] = info[k];
    
                }
            }
        }
    }
    pthread_mutex_unlock(&data_mutex);

    char message[MAXLENGTH];
    size_t message_size = serverConstructingMessage(message, text, number_of_mes * 3 + 1);

    /*printf("\nserver message:\n");

    for(k = 0; k < message_size; k++){
        printf("%X ", message[k]);
    }
    printf("\n");*/

    ssize_t cntSending = send(socket, message, message_size, 0);
    if (cntSending != message_size) {
        fprintf(stderr, "Can't send history to %s\n", socket);
    }
    pthread_mutex_unlock(&data_mutex);
}

int statusK(CClientStr* client_str, CServerMood* server_mood, int socket, CUserInfo* user_info, int kick_id) {
    //printf("statusK\n");
    char* login;
    int login_id;
    int length;
    int i;
    int kickClients = 0;
    ssize_t cntSend;

    char text[NUMBEROFSTR][MINLENGTH];
    char timeStr[TIMELENGTH];
    getTime(timeStr);
    strcpy(text[0], "k");
    char* reason = client_str->text[1];
    strcpy(text[1], reason);
    char message[MAXLENGTH];

    size_t message_size = serverConstructingMessage(message, text, 2);
    
    /*printf("\nstatusK:\n");
    for ( i = 0; i < message_size; i++){
        printf("%X ", message[i]);
    }*/
    //pthread_mutex_lock(&data_mutex);

    for (i = 0; i < server_mood->number_of_users; ++i) {
        if (server_mood->users[i].user_id == kick_id &&
                server_mood->users[i].online) {
            server_mood->users[i].online = 0;
            login = server_mood->users[i].login;
            login_id = server_mood->users[i].user_id;
            socket = server_mood->users[i].socket_id;
            //printf("to_kick: %s %d %d %d\n", login,socket, login_id, server_mood->threads[server_mood->users[i].thread_id]);
            cntSend = send(socket, message, message_size, 0);
            close(socket);
            pthread_cancel(server_mood->threads[server_mood->users[i].thread_id]);
            if (cntSend != message_size) {
                fprintf(stderr, "Can't send kick message\n");
            }
            kickClients += 1;
        }
    }
    if (!kickClients) {
        return 2;
    } 
    return 0;
}

void generateMMes(CServerMood* server_mood, char reason, char* login, int user_id) {
    //printf("generateMMes\n");
    char reason_str[MAXLENGTH];
    reason_str[0] = 0;
    int length = 0;
    int total_lenght = 0;
    int i;

    switch(reason) {
        case'i':
            strcpy(reason_str, "' enter ");
            break;
        case'o':
            strcpy(reason_str, "' log out from ");
            break;
        case'k':
            strcpy(reason_str, "' was kicked from ");
            break;
        case 'd':
            strcpy(reason_str, "' disconnected from ");
            break;
        default:
            break;
    }

    char str[MAXLENGTH]; 
    char id[MAXLENGTH];
    IntToStr(id, user_id);
    str[0] = 0;
    strcat(str, "User with id: ");
    strcat(str, id);
    strcat(str, " and login: '");
    strcat(str, login);
    strcat(str, reason_str);
    strcat(str, "this shiny chat!");



    char text[NUMBEROFSTR][MINLENGTH];
    char timeStr[TIMELENGTH];
    getTime(timeStr);
    strcpy(text[0], "m");
    memcpy(text[1], timeStr, 8);
    text[1][9] = 0;
    strcpy(text[2], str);
    char message[MAXLENGTH];
    size_t message_size = serverConstructingMessage(message, text, 3);

   
    /*printf("\nM message7: \n");
    for(i = 0; i < message_size; i++){
        printf("%X ", message[i]);
    }
    printf("\n");
`   */
    sendMessageToOnlineUsers(server_mood, message, message_size);
}

int readClientString(char* string, int length, int socket, int thread, CServerMood* server_mood, CUserInfo* user_info) {
    int number_of_mes = 0;
    CClientStr client_str;
    client_str.number_of_str = 0;

    int j;
    int i = 0;
    int str_number = 0;
    int total_lenght = 0;
    int kick_id = 0;
    /*printf("Reading from client %d:\n", socket);
    for (j = 0; j < length; j++) {
        printf("%X ", string[j]);
    }
    printf("\n------------------------\n");*/
    
    client_str.status = string[0];
    //printf("message status: %c\n", client_str.status);

    total_lenght = getTotalLength(string);

    //printf("total_lenght: %d lenght: %d\n", total_lenght, length);

    int flagH = 0;
    if (string[0] == 'h'){
        flagH = 1;
        //printf("flagH: %d\n", flagH);
    }

    int flagK = 0;
    if (string[0] == 'k'){
        flagK = 1;
        //printf("flagK: %d\n", flagK);
    }

    // Форматирование строки
    while(i < total_lenght) {
        str_number = client_str.number_of_str;
        if (client_str.number_of_str > 2) {
            return 6;
        }

        if (flagH){
            //printf("total_lenght: %d\n", total_lenght);
            number_of_mes = getLength(getStringFromMessage(string, str_number));
            client_str.number_of_str += 1;
            str_number += 1;
            //printf("number_of_mes: %d\n", number_of_mes);
            break;
        }

        if (flagK){
            //printf("total_lenght: %d\n", total_lenght);
            kick_id = getLength(getStringFromMessage(string, str_number));
            str_number += 1;
            //printf("kick_id: %d\n", kick_id);
            i += 4;
            i += 4;
        }

        int len = getStringLenghtFromMessage(string, str_number);
        //printf("len: %d\n", len);
        i += 4;
        i += len;
        strncpy(client_str.text[str_number], getStringFromMessage(string, str_number), len);
        client_str.text[str_number][len] = 0;
        client_str.number_of_str += 1;
        //printf("total_lenght: %d i: %d\n", total_lenght, i);
        //printf("client_str.text[client_str.number_of_str - 1]: %s\n", client_str.text[str_number]);
    }

    int temp;
    char info = NULL;
    switch(client_str.status) {
        case'r':
            if (!user_info->online) {
                return 2;
            }  
            temp = statusR(&client_str, server_mood, socket, user_info);
            if (temp != 0){
                return temp;
            }
            break;
        case'i':
            // Если зарегестрированный пользователь хочет зарегестрироваться ещё раз.
            if (user_info->online) {
                return 4;
            }
            temp = statusI(&client_str, server_mood, socket, thread, user_info);
            if (temp != 0){
                return temp;
            }
            user_info->online = 1;
            generateMMes(server_mood, 'i', user_info->login, user_info->user_id);
            break;
        case'o':
            if (!user_info->online) {
                return 2;
            }
            temp = statusO(&client_str, server_mood, socket, user_info);
            if (temp != 0){
                return temp;
            }
            generateMMes(server_mood, 'o', user_info->login, user_info->user_id);
            user_info->online = 0;
            break;
        case'h':
            if (!user_info->online) {
                return 2;
            }
            temp = statusH(&client_str, server_mood, socket, user_info, number_of_mes);
            if (temp != 0){
                return temp;
            }
            break;
        case'l':
            if (!user_info->online) {
                return 2;
            }
            temp = statusL(&client_str, server_mood, socket, user_info);
            if (temp != 0){
                return temp;
            }
            break;
        case'k':
            if (!user_info->online) {
                return 2;
            }
            temp = statusK(&client_str, server_mood, socket, user_info, kick_id);
            if (temp != 0){
                return temp;
            }
            char* kick_login;
            for (i = 0; i < server_mood->number_of_users; ++i) {
                if (server_mood->users[i].user_id == kick_id) {
                    kick_login = server_mood->users[i].login;
                }
            }
            generateMMes(server_mood, 'k', kick_login, kick_id);
            break;
        default:
            return 1;
            break;
    }
    //printf("\n------------------------\n");

    //printf("%d\n", server_mood->number_of_users );

    /*for (j = 0; j < server_mood->number_of_users; j++) {
        printf("information about users: socket_id = %d, online = %d, user_id = %d, thread_id = %d, login = %s\n", 
            server_mood->users[j].socket_id, server_mood->users[j].online, 
            server_mood->users[j].user_id, server_mood->threads[server_mood->users[j].thread_id],
            server_mood->users[j].login);
    }*/

    return 0;
}