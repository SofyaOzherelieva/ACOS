#include "init_destroy.h"

void initialize_program(program* prog, char* word) {
    prog->name = (char*)malloc(strlen(word) + 1);
    memcpy(prog->name, word, strlen(word) + 1);
    prog->arg_number = 0;
    prog->output_file = NULL;
    prog->output_type = 0;
    prog->arguments = (char**)malloc(2 * sizeof(char*));
    prog->arguments[1] = NULL;
    prog->input_file = NULL;
    prog->arguments[0] = (char*)malloc(strlen(word) + 1);
    memcpy(prog->arguments[0], word, strlen(word) + 1);
}


void destroy_prog(program* prog) {
    if (!prog) {
        return;
    }
    free(prog->name);
    free(prog->input_file);
    free(prog->output_file);
    int k;
    for (k = 0; k <= prog->arg_number; ++k) {
        free(prog->arguments[k]);
    }
    free(prog->arguments);
}

void finish_work(int exit_flag) {
    int i, j, k;
    for (i = 0; i < jobs_number; ++i) {
        if(exit_flag){
            if (!kill(jobs[i].pid, SIGINT)) {
                kill(jobs[i].pid, SIGINT);
            }
        }
        if (jobs[i].name != NULL) {
            free(jobs[i].name);
        }
        for (j = 0; j < jobs[i].number_of_programs; ++j) {
            destroy_prog( &jobs[i].programs[j] );
        }
        if (jobs[i].number_of_programs != 0) {
            free(jobs[i].programs);
        }
    }
}

int set_environment_vars() {
    struct passwd *pw;
    pw = getpwuid(getuid());
    if (!pw) {
        return ENV_VARS_FAIL;
    }
    char* dir = pw->pw_dir;

    int status = setenv("HOME", dir, 1);
    if (status == -1) {
        return ENV_VARS_FAIL;
    }

    status = setenv("USER", pw->pw_name, 1);
    if (status == -1) {
        return ENV_VARS_FAIL;
    }
    // Устанавливаем домашнюю директорию, как начальную.
    status = setenv("PWD", getenv("HOME"), 1);
    if (status == -1) {
        return ENV_VARS_FAIL;
    }

    status = setenv("OLDPWD", getenv("HOME"), 1);
    if (status == -1) {
        return ENV_VARS_FAIL;
    }

    char symlink[MAX_LENGTH + 1];
    char shell[MAX_LENGTH + 1];

    int len = readlink(symlink, shell, MAX_LENGTH);
    shell[len] = '\0';

    status = setenv("SHELL", shell, 1);
    if (status == -1) {
        return ENV_VARS_FAIL;
    }

    return 0;
}

void initialize_prog(char* query) {
    jobs[jobs_number].name = (char*)malloc(strlen(query) * sizeof(char) + 1);
    jobs[jobs_number].number_of_programs = 0;
    memcpy(jobs[jobs_number].name, query, strlen(query));
    jobs[jobs_number].name[strlen(query)] = '\0';
    jobs[jobs_number].number_of_programs = 0;
    jobs[jobs_number].working = 0;
    jobs[jobs_number].programs = NULL;
}