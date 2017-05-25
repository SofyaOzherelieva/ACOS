#include "commands.h"


void change_dir_simple(const char* mod) {
    int i, j;
    if (mod == NULL) {
        for (i = (int)strlen(curr_dir) - 1; i >= 0; --i) {
            curr_dir[i] = '\0';
        }
        memcpy(curr_dir, home_dir, strlen(home_dir));
        return;
    }
    if (!strcmp(mod, "/")) {
        int i;
        for (i = (int)strlen(curr_dir) - 1; i >= 0; --i) {
            curr_dir[i] = '\0';
        }
        curr_dir[0] = '/';
        return;
    }
    if (!strcmp(mod, "-")) {
        int i;
        if (!strcmp(curr_dir, "/")) {
            return;
        }
        for (i = (int)strlen(curr_dir) - 1; ; --i) {
            if (curr_dir[i] == '/') {
                curr_dir[i] = '\0';
                break;
            }
            curr_dir[i] = '\0';
        }
        if ((int)strlen(curr_dir) == 0) {
            curr_dir[0] = '/';
        }
        return;
    }
    if (curr_dir[strlen(curr_dir) - 1] != '/') {
        curr_dir[strlen(curr_dir)] = '/';
    }
    for (j = 0, i = (int)strlen(curr_dir); j < (int)strlen(mod); ++j, ++i) {
        curr_dir[i] = mod[j];
    }
}

void change_dir(const char* mod) {
    int i = 0;
    if (mod == NULL) {
        change_dir_simple(NULL);
        return;
    }
    if (mod[0] == '/') {
        change_dir_simple("/");
        i = 1;
    }
    for (; i < (int)strlen(mod);) {
        int j;
        char* tmp;
        for (j = i; j < (int)strlen(mod); ++j) {
            if (mod[j] == '/') {
                break;
            }
        }
        tmp = (char*)malloc(j - i + 1);
        memcpy(tmp, mod + i, j - i);
        tmp[j - i] = '\0';
        change_dir_simple(tmp);
        free(tmp);
        i = j + 1;
    }
}

// Если вводишь несуществующую директорию, то почему-то всё нормально((
int cd_execute(program* prog) {
    char *path;
    char *path_real;
    if (prog->arg_number == 0) {
        path = getenv("HOME");
    } else if (prog->arg_number == 1) {
        path = prog->arguments[1];
    } else {
        return TOO_MANY_ARGUMENTS;
    }

    //printf("\npath: %s\n", path);
    if(!strcmp(path, "-")) {
        path_real = getenv("OLDPWD");
        //printf("path_real: %s\n", path_real);
    } else {
        path_real = realpath(path, NULL);
        if (!path_real) {
            free(path_real);
            return CD_FAIL;
        }
    }
    DIR* dir;
    dir = opendir(prog->arguments[0]);
    if (dir == NULL)
    {
        return NO_SUCH_DIRECTORY;
    }
    closedir(dir);    

    printf("path_real: %s\n", path_real);
    if (!chdir(path_real)) {
        free(path_real);
        return CD_FAIL;
    }

    if (setenv("OLDPWD", getenv("PWD"), 1) == -1) {
        free(path_real);
        return CD_FAIL;
    }

    if (setenv("PWD", path_real, 1) == -1) {
        free(path_real);
        return CD_FAIL;
    }

    free(path_real);
    return 0;
}

int write_history(program* prog)
{
    int i;
    if (prog->arg_number != 0) {
        return TOO_MANY_ARGUMENTS;
    }
    for (i = 0; i < jobs_number; ++i) {
        fprintf(stdout, " %d: %s", i, jobs[i].name);
        fflush(stdout);
    }
    return 0;
}