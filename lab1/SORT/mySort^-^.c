#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

#define MAXLENGTH 4096


static sort(int iflag, FILE* input_file, int lflag, int rflag) {
    int j = 0;
    int sizeof_text = 0;
    int i = 0;
    char *text[MAXLENGTH];
    size_t len = 0;
    ssize_t read;
    int reverse_multiplier = 1;
    while((read = getline(&text[i], &len, input_file)) != -1) {
        i++;
    }
    sizeof_text = i;

    if(rflag == 1) {
    	reverse_multiplier = -1;
    }

    // lexicographic
    if (lflag == 1) {
        for (i = 0; i < sizeof_text; i++) {
            for (j = i; j < sizeof_text; j++) {
                if ((strcmp(text[i], text[j]) * reverse_multiplier) > 0) {
                    char* tmp = text[i];
                    text[i] = text[j];
                    text[j] = tmp;
                }
            }
        }
    }

    // reverse the result of comparisons
    if (rflag == 1 && lflag == 0) {
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
        sort(iflag, stdin, lflag, rflag);
    }
}
