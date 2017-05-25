#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <dirent.h>
#include <signal.h>
#include <pwd.h>

#include "error.h"
#include "init_destroy.h"
#include "parse.h"

void handler(int sig) {
    if (sig == SIGINT) {
        finish_work(0);
        exit(0);
    }
}

// Отладка.
void debug_output(program* prog) {
    int i;
    printf("Program name: %s\n", prog->name);
    printf("Number of arguments: %d\n", prog->arg_number);
    for (i = 0; i <= prog->arg_number; ++i) {
        printf("Argument №%d: %s\n", i, prog->arguments[i]);
    }
    printf("Input file: %s\n", prog->input_file);
    printf("Output file: %s\n", prog->output_file);
    printf("Output type: %d\n", prog->output_type);
    fflush(stdout);
}

int execute_job(int job_id)
{
    int i, j;
    int** pipes;
    chdir(curr_dir);
    pipes = NULL;
    if (jobs[job_id].number_of_programs > 1)
    {
        pipes = (int**)malloc((jobs[job_id].number_of_programs - 1) * sizeof(int*));
    }
    // Создаем конвейер.
    for (i = 0; i < jobs[job_id].number_of_programs - 1; ++i)
    {
        pipes[i] = (int*)malloc(2 * sizeof(int));
        pipe(pipes[i]);
    }
    for (i = 0; i < jobs[job_id].number_of_programs; ++i)
    {
        int status;
        int child;
        if (!strcmp(jobs[job_id].programs[i].name, "cd"))
        {
            int fd_stdin = -1;
            int fd_stdout = -1;
            if (i != 0)
            {
                fd_stdin = dup(0); //stdin
                // ассоциация pipe0 и stdin
                dup2(pipes[i - 1][0], 0); 
                close(pipes[i - 1][0]);
            }
            if (i != jobs[job_id].number_of_programs - 1)
            {
                fd_stdout = dup(1); // stdout
                // ассоциация pipe1 и stdout
                dup2(pipes[i][1], 1);
                close(pipes[i][1]);
            }
            status = cd_execute(&jobs[job_id].programs[i]);
            if (status != 0)
            {
                jobs[job_id].working = 0;
                handler(SIGINT);
                return status;
            }
            if (fd_stdin != -1)
            {
                dup2(fd_stdin, 0);
            }
            if (fd_stdout != -1)
            {
                dup2(fd_stdout, 1);
            }
            continue;
        }
        if (!strcmp(jobs[job_id].programs[i].name, "history"))
        {
            int fd_stdin = -1;
            int fd_stdout = -1;
            if (i != 0)
            {
                fd_stdin = dup(0);
                dup2(pipes[i - 1][0], 0);
                close(pipes[i - 1][0]);
            }
            if (i != jobs[job_id].number_of_programs - 1)
            {
                fd_stdout = dup(1);
                dup2(pipes[i][1], 1);
                close(pipes[i][1]);
            }
            status = write_history(&jobs[job_id].programs[i]);
            if (status != 0)
            {
                jobs[job_id].working = 0;
                handler(SIGINT);
                return status;
            }
            if (fd_stdin != -1)
            {
                dup2(fd_stdin, 0);
            }
            if (fd_stdout != -1)
            {
                dup2(fd_stdout, 1);
            }
            continue;
        }
        child = fork();
        if (child == 0)
        {
            char** param = (char**)malloc((jobs[job_id].programs[i].arg_number + 2) * sizeof(char*));
            for (j = 0; j <= jobs[job_id].programs[i].arg_number; ++j)
            {
                param[j] = (char*)malloc(strlen(jobs[job_id].programs[i].arguments[j]) * sizeof(char) + sizeof(char));
                memcpy(param[j], jobs[job_id].programs[i].arguments[j], strlen(jobs[job_id].programs[i].arguments[j]));
                param[j][strlen(jobs[job_id].programs[i].arguments[j])] = '\0';
            }
            param[jobs[job_id].programs[i].arg_number + 1] = NULL;
            if (i != 0)
            {
                dup2(pipes[i - 1][0], 0);
                close(pipes[i - 1][0]);
            }
            if (i != jobs[job_id].number_of_programs - 1)
            {
                dup2(pipes[i][1], 1);
                close(pipes[i][1]);
                close(pipes[i][0]);
            }
            for (j = i + 1; j < jobs[job_id].number_of_programs - 1; ++j)
            {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }
            for (j = 0; j < jobs[job_id].number_of_programs - 1; ++j)
            {
                free(pipes[j]);
            }
            free(pipes);
            execvp(jobs[job_id].programs[i].name, param);
        }
        if (i != 0)
        {
            close(pipes[i - 1][0]);
        }
        if (i != jobs[job_id].number_of_programs - 1)
        {
            close(pipes[i][1]);
        }
        waitpid(child, &status, WUNTRACED);
        if (status != 0)
        {
            jobs[job_id].working = 0;
            handler(SIGINT);
            return status;
        }
    }
    jobs[job_id].working = 0;
    for (i = 0; i < jobs[job_id].number_of_programs - 1; ++i)
    {
        free(pipes[i]);
    }
    free(pipes);
    handler(SIGINT);
    return 0;
}

void sigint_handler(int sig)
{
    if (getpid() != leader_pid && jobs[getpid()].working) {
        raise(SIGINT);
    }
    if (jobs[current_id].working) {
        jobs[current_id].working = 0;
        kill(jobs[current_id].pid, SIGINT);
    }
}

int main()
{
    jobs_number = 0;
    int i;
    struct sigaction chld_sa;
    int status = 0;

    signal(SIGINT, sigint_handler);
    
    // Определяем значения служебных переменных.
    status = set_environment_vars();
    if (status) {
        my_perror(status);
    }
    memcpy(curr_dir, getenv("PWD"), strlen(getenv("PWD")));
    memcpy(home_dir, getenv("HOME"), strlen(getenv("HOME")));

    leader_pid = getpid();

    while (1) {
        char query[MAX_QUERY_SIZE];
        int i, j;
        int child_pid;
        memset(query, 0, MAX_QUERY_SIZE);

        fprintf(stdout, "%s$ ", curr_dir);
        fflush(stdout);
        if (fgets(query, MAX_QUERY_SIZE, stdin) == NULL) {
            fprintf(stdout, "EOF. End of work\n");
            fflush(stdout);
            finish_work(1);
            return 0;
        }
        if (!strcmp(query, "exit\n")) {
            fprintf(stdout, "Exit. End of work\n");
            fflush(stdout);
            finish_work(1);
            return 0;
        }

        // Удаляем комментарии
        for(i = 0; i < strlen(query); i++) {
            if (query[i] == '#') {
                query[i] = '\0';
            }
        }

        initialize_prog(query);

        for (i = 0; i < (int)strlen(jobs[jobs_number].name);) {
            for (j = i; j < (int)strlen(jobs[jobs_number].name); ++j) {
                if (jobs[jobs_number].name[j] == '|') {
                    break;
                }
            }
            ++jobs[jobs_number].number_of_programs;
            jobs[jobs_number].programs = (program*)realloc(jobs[jobs_number].programs, jobs[jobs_number].number_of_programs * sizeof(program));
            parse(i, query, &jobs[jobs_number].programs[jobs[jobs_number].number_of_programs - 1]);
            i = j + 1;
        }
        for (i = 0; i < jobs[jobs_number].number_of_programs; ++i) {
            debug_output(&jobs[jobs_number].programs[i]);
        }
        jobs[jobs_number].working = 1;


        child_pid = fork();
        if (child_pid == -1)
        {
            perror("fork");
            exit(1);
        }
        if (child_pid == 0) {
            signal(SIGINT, handler);
            child_pid = getpid();
            jobs[jobs_number].pid = child_pid;
            execute_job(jobs_number);
            if (status) {
                my_perror(status);
            }
        }

        jobs[jobs_number].pid = child_pid;

        current_id = jobs_number;
        waitpid(child_pid, &status, 0);
        if (jobs[jobs_number].number_of_programs == 1 && !strcmp(jobs[jobs_number].programs[0].name, "cd") && !status) {
            change_dir(jobs[jobs_number].programs[0].arguments[1]);
        }
        ++jobs_number;
    }

    return 0;
}