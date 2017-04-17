#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>


enum operation {FILE_TO_FILE, FILE_TO_DIR};

static int copyFILE_TO_FILE(char* source, char* destination) {
    FILE* copyFrom = fopen(source, "r");

    if (copyFrom == NULL) {
        fprintf(stderr, "Open file failed\n");
        return 1;
    }
    
    FILE* copyTo = fopen(destination, "w");

    // If there is no such file, we will create it.
    if (copyTo == NULL) {
        copyTo = fopen(destination, "wb");
        if (copyTo == NULL) {
            fprintf(stderr, "Create file failed\n");
            return 2;
        } 
    }
    int caractereActuel = fgetc(copyFrom);
    for (; caractereActuel != EOF; caractereActuel = fgetc(copyFrom)) {
        fputc(caractereActuel, copyTo);
    }
    fclose(copyFrom);
    fclose(copyTo);
    return 0;
}

static int copyFILE_TO_DIR(char* source, char* destination) {
    FILE* copyFrom = fopen(source, "r" );

    if (copyFrom == NULL) {
        fprintf(stderr, "Open file failed\n");
        return 1;
    }

    char newFileName[256] = {0};
    sprintf(newFileName, "%s%s%s", destination, "/",source);

    FILE* copyTo = fopen(newFileName, "w");

    // If there is no such file, we will create it.
    if (copyTo == NULL) {
        copyTo = fopen(newFileName, "wb");
        if (copyTo == NULL) {
            fprintf(stderr, "Create file failed\n");
            return 2;
        } 
    }
    int caractereActuel = fgetc(copyFrom);
    for (; caractereActuel != EOF; caractereActuel = fgetc(copyFrom)) {
        fputc(caractereActuel, copyTo);
    }
    fclose(copyFrom);
    fclose(copyTo);
    return 0;
}


int main(int argc, char *argv[]) {
    struct stat source_stat, destination_stat;
    enum operation type;

    char* destination = argv[--argc];
    argv[argc] = NULL;
    char* source = argv[--argc];
    argv[argc] = NULL;
    int r = stat(destination, &destination_stat);
    if (r == -1 || !S_ISDIR(destination_stat.st_mode)) {
        type = FILE_TO_FILE;
    } else
        type = FILE_TO_DIR;

    if (type == FILE_TO_FILE){
        exit (copyFILE_TO_FILE(source, destination));
    }
    if (type == FILE_TO_DIR){
        exit (copyFILE_TO_DIR(source, destination));
    }
}
