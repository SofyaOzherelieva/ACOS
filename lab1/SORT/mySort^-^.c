#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

#define MAXLENGTH 4096

int str_cmp(const char* str1, const char* str2) {
    int min_len = 0;
    if (strlen(str1) > strlen(str2)) {
        min_len = strlen(str2);
    } else {
        min_len = strlen(str1);
    }
    int i = 0;
    for (i = 0; i < min_len; i++) {
        if (str1[i] > str2[i]) {
            return -1;
        }
        if (str1[i] < str2[i]) {
            return 1;
        }
    }
    if (strlen(str1) > strlen(str2)) {
        return 1;
    }
    if (strlen(str1) < strlen(str2)) {
        return -1;
    }
    return 0;
}


static sort(int iflag, FILE* input_file, int lflag, int rflag) {
    int j = 0;
    int sizeof_text = 0;
    int i = 0;
    char* text[MAXLENGTH];
    char string[MAXLENGTH];
    // Input.
    if (iflag == 0) {
        for (i = 0; i < MAXLENGTH;) {
            if (fgets(string, sizeof(string), stdin) == NULL) {
                break;
            }
            text[i++] = strdup(string);
        }
        sizeof_text = i;
    } else {
        for (i = 0; i < MAXLENGTH;) {
            if (fgets(string, sizeof(string), input_file) == NULL) {
                break;
            }
            text[i++] = strdup(string);
        }
        sizeof_text = i;
    }

    // lexicographic
    if (lflag == 1) {
        for (i = 0; i < sizeof_text; i++) {
            for (j = i; j < sizeof_text; j++) {
                if (str_cmp(text[i], text[j]) < 0) {
                    char* tmp = text[i];
                    text[i] = text[j];
                    text[j] = tmp;
                }
            }
        }
    }

    // reverse the result of comparisons
    if (rflag == 1) {
        for (i = 0; i < sizeof_text; i++) {
            if (sizeof_text - 1 - i > i) {
                char* tmp = text[sizeof_text - 1 - i];
                text[sizeof_text - 1 - i] = text[i];
                text[i] = tmp;
            }
        }
    }
    for (i = 0; i < sizeof_text; i++) {
        printf("%s", text[i]);
    }
}

static const char *optString = "i:lr?";

int main(int argc, char *argv[]) {
    int lflag = 0, rflag = 0, iflag = 0;
    int options = getopt(argc, argv, optString);
    FILE* input_file = NULL;

    while(options != -1) {
        switch(options) {
        case 'l': // lexicographic
        lflag = 1;
            break;
        case 'r': // reverse
            rflag = 1;
            break;
        case 'i': // file or std input
            iflag = 1;
            input_file = fopen(optarg, "rw");
            if (input_file == NULL) {
                fprintf(stderr, "Open file failed\n");
                return 1;
            }
            break;
        default:
            break;
        }
        options = getopt(argc, argv, optString);
    }
    if (iflag == 1) {
        sort(iflag, input_file, lflag, rflag);
        fclose(input_file);
    } else {
        sort(iflag, input_file, lflag, rflag);
    }
}
