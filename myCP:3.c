#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>


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
    FILE* copyTo = fopen(destination,"w");
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
	char newFileName[MAX] = {0};
	sprintf(newFileName, "%s/%s", *destination, copyFrom);
	FILE* copyTo  = fopen(newFileName, "w");
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