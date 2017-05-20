#include "receive_mes.h"

void* processConnection(void* args) {
    CUserInfo user_info;
    user_info.online = 0;
    user_info.user_id = 0;
    user_info.login;
    user_info.password;

    CSomeArgs *arg = (CSomeArgs*) args;
    char buffer[BUF_SIZE];

    char* message;
    int* socket_desc = arg->id;
    CServerMood* server_mood = arg->current;

    int socket = *(int*)socket_desc;
    int thread = arg->thread_id;


    if (server_mood->root == 0) {
            server_mood->root = socket;
    }
    //printf("root: %d\n", server_mood->root);


    int j = 0;
    int i = 0;
    for (;;) {

        int count = recv(socket, buffer, BUF_SIZE, 0);
        
        /*printf("%d\n", count);
        printf("\nbuffer: \n");
        for ( i = 0; i < count; i++) {
            printf("%X ", buffer[i]);
        }
        printf("\n");

        printf("%d %d\n", sizeof(buffer), strlen(buffer));*/

        if (count == -1) {
            fprintf(stderr, "recv() failed\n");
            free(socket_desc);
            return;
        }

        if (count == 0) { // Если посылается ^C )
            generateMMes(server_mood, 'd', user_info.login, user_info.user_id);
            fprintf(stdout, "Client disconnected\n");
            user_info.online = 0;
            server_mood->users[user_info.user_id].online = 0;
            free(socket_desc);
            return;
        }

        // ПОПРАВИТЬ

        //count -= 1; // Не учитываем последнее /0. // Должны не посылать последний ноль!!!!!
        char * string = (char*) malloc(count * sizeof(char));
        int u = 0;
        for (u = 0; u < count; u++) {
            string[u] = buffer[u];
        }

        int result = readClientString(string, count, socket, thread, server_mood, &user_info);

        //printf("result: %d\n", result);

        if (result) {
            generateStatusMes(socket, result);
        }

        //printf("Answer to client %d:\n", socket);

        free(string);
    }
    free(socket_desc);
    return;
    //close(socket); // Так можно кикнуть))
}
