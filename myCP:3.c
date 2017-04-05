#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#define MAX 1024

static int copyFILE_TO_FILE(char* source, char* destination);
static int copyFILE_TO_DIR(char* source, char* destination);

enum operation { FILE_TO_FILE, FILE_TO_DIR};

int main(int argc, char *argv[])
{
	struct stat source_stat, destination_stat; 
	enum operation type;

	char* destination = argv[--argc];
	argv[argc] = NULL;
	char* source = argv[--argc];
	argv[argc] = NULL;
	int r = stat(destination, &destination_stat);

	if (r < 0) {
        fputs ("Stat error",stderr); 
        exit(1);
    }

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

static int copyFILE_TO_FILE(char* source, char* destination)
{
	FILE* copyFrom = fopen(source,"r");

	if (copyFrom == NULL) {
        fputs ("Copy_from file open error",stderr); 
        exit(2);
    }

    FILE* copyTo = fopen(destination,"w");

    if (copyTo == NULL) {
        fputs ("Copy_to file open error",stderr); 
        exit(3);
    }

    for (;;) {
        int caractereActuel = fgetc(copyFrom);
        if (caractereActuel != EOF) {
           	fputc(caractereActuel,copyTo);
        }
        else {
           	break;
        }
    }
    fclose(copyFrom);
    fclose(copyTo);
    return 0;
}
static int copyFILE_TO_DIR(char* source, char* destination)
{
	FILE* copyFrom = fopen(source,"r");

	if (copyFrom == NULL) {
        fputs ("Copy_from file open error",stderr); 
        exit(2);
    }

	char newFileName[MAX] = {0};
	int r = sprintf(newFileName, "%s/%s", *destination, copyFrom);

	if (r < 0) {
        fputs ("Sprintf error",stderr); 
        exit(4);
    }

	FILE* copyTo  = fopen(newFileName, "w");

	if (copyTo == NULL) {
        fputs ("Copy_to file open error",stderr); 
        exit(3);
    }

	for (;;) {
        int caractereActuel = fgetc(copyFrom);
        if (caractereActuel != EOF) {
           	fputc(caractereActuel,copyTo);
        }
        else {
           	break;
        }
    }
    fclose(copyFrom);
    fclose(copyTo);
	return 0;
}
