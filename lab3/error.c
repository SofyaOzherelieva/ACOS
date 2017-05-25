#include "error.h"

void my_perror(int error) {
    if (error == TOO_MANY_ARGUMENTS) {
        fprintf(stderr, "Too many arguments\n");
    }
    if (error == NO_SUCH_DIRECTORY) {
        fprintf(stderr, "No such directory\n");
    }
    if (error == CD_FAIL) {
        fprintf(stderr, "CD failed\n");
    }
    if (error == 0) {
        fprintf(stderr, "All done\n");
    }
    if (error == ENV_VARS_FAIL) {
        fprintf(stderr, "Environment variables setting failed\n");
    }
}