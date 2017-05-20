#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>

#define MAXLENGTH 70000
#define MINLENGTH 1000

void printTime(time_t sec) {
    struct tm* timeInfo = localtime(&sec);
    printf("%s", asctime(timeInfo));
}

char* intToBigEndian(char* str, unsigned long number) {
    int i;
    for (i = 3; i >= 0; i--) {
        str[i] = (char)(number % 256);
        number /= 256;
    }
}

void IntToStr(char* buff, int x) {
    if (x != 0) {
        int y = 1;
        while (y <= x) {
            y = y * 10;
        }
        y = y / 10;
        int i;
        for (i = 0; y; ++i) {
            buff[i] = (char) ('0' + (x / y));
            x = x - y * (x / y);
            y = y / 10;
        }
        buff[i] = 0;
    } else {
        buff[0] = '0';
        buff[1] = 0;
    }
}

void getTime(char* time_str) {
    struct timeval t;
    gettimeofday(&t, NULL);
    intToBigEndian(time_str, (size_t)t.tv_sec);
    intToBigEndian(time_str + 4, (size_t)t.tv_usec);
    time_str[9] = '\0';
}

size_t getLength(char* message) {
    int i;
    size_t length = 0;
    for (i = 0; i < 4; ++i) {
        length = length * 256 + (message[i] + 256) % 256;
    }
    return length;
    /*size_t length = (message[0] << 24) | (message[1] << 16) | (message[2] << 8) | message[3];
    return length;*/
}

size_t getStringLenghtFromMessage(char* message, size_t index) {
    int i;
    message += 5;
    size_t length = 0;
    for (i = 0; i <= index; ++i) {
        length = getLength(message);
        message += 4;
        message += length;
    }
    return length;
}

char* getStringFromMessage(char* message, int index) {
    int i;
    message += 5;
    size_t length = 0;
    for (i = 0; i < index; ++i) {
        length = getLength(message);
        message += 4;
        message += length;
    }
    message += 4;
    return message;
}

size_t getTotalLength(char* message) {
    message += 1;
    size_t length = getLength(message);
    return length;
}

size_t serverConstructingMessage(char* string, char text[][MINLENGTH], int number_of_str) {
    //printf("serverConstructingMessage\n");

    int i = 0;
    int j = 0;
    int flag = 0; // Учитываем символ конца строки(не записывам его)
    size_t string_length = 0;
    int flagRMH = 0;

    if (text[0][0] == 'r' || text[0][0] == 'm' || text[0][0] == 'h') {
        flagRMH = 1;
    }
    //printf("flagRMH %d\n", flagRMH);

    string[0] = text[0][0];
    string_length += 1;

    intToBigEndian(string + string_length, 0);

    string_length += 4;
    /*if (flagRMH) {
        intToBigEndian(string + string_length, 8);
        string_length += 4;
        for (i = 0; i <8; i++) {
            string[string_length + i] = text[flagRMH][i];
        }
        string_length += 8;
    }*/

    for (i = 1; i < number_of_str; i++) {
        flag = 0;
        //printf("i / 3 %d\n", i % 3);
        if (flagRMH && (i % 3 == 1)) {
            //printf("time\n");
            intToBigEndian(string + string_length, 8);
            string_length += 4;
            for (j = 0; j < 8; j++) {
                string[string_length + j] = text[i][j];
            }
            string_length += 8;
        } else {
            if (text[i][strlen(text[i]) - 1] == '\n') {
                flag = 1;
            };
            intToBigEndian(string + string_length, strlen(text[i]) - flag);
            string_length += 4; 

            for (j = 0; j < strlen(text[i]) - flag; j++) {
                string[j + string_length] = text[i][j];
            }
            string_length += strlen(text[i]) - flag;
        }
    }

    intToBigEndian(string + 1, string_length - 5);

    string[string_length + 1] = '\0';

    /*printf("\nserverConstructingMessage:\n");
    for (i = 0; i < string_length; i++) {
        printf("%X ", string[i]);
    }*/

    return string_length;
}

int min(int x, int y) {
    if (x < y) {
        return x;
    } else {
        return y;
    }
}


size_t clientConstructingMessage(char* string, char* text[], int number_of_str) {

    int i = 0;
    int j = 0;
    int flagKH = 0;
    int flag = 0; // Учитываем символ конца строки(не записывам его)
    size_t string_length = 1;
    //printf("status: %s\n", text[0]);

    //printf("\nclientConstructingMessage:\n");

    string[0] = text[0][0];

    intToBigEndian(string + string_length, 0);
    string_length += 4;

    if (string[0] == 'k' || string[0] == 'h') {
        flagKH = 1;
    }

    /*printf("\nclientConstructingMessage:\n");
    for (i = 0; i < string_length; i++) {
        printf("%X ", string[i]);
    }*/

    for (i = 1; i < number_of_str; i++) {
        if (flagKH && i == 1) {
            intToBigEndian(string + string_length, 4);
            string_length += 4;
            for (j = 0; j < 4; j++) {
                string[j + string_length] = text[i][j];
            }
            string_length += 4;
        } else {
            flag = 0;
            if (text[i][strlen(text[i]) - 1] == '\n') {
                flag = 1;
            };
            intToBigEndian(string + string_length, strlen(text[i]) - flag);
            string_length += 4;
            for (j = 0; j < strlen(text[i]) - flag; j++) {
                string[j + string_length] = text[i][j];
            }
            string_length += strlen(text[i]) - flag;
        }
        /*printf("\nclientConstructingMessage:\n");
        for (j = 0; j < string_length; j++) {
            printf("%X ", string[j]);
        }*/
    }

    while (string[string_length - 1] == 0) {
        string_length--;
    }

    //printf("total_lenght%d \n", string_length - 5);
    intToBigEndian(string + 1, string_length - 5);

    /*printf("\nclientConstructingMessage:\n");
    for (i = 0; i < string_length; i++) {
        printf("%X ", string[i]);
    }*/

    //printf("\n");

    return string_length;
}
 
