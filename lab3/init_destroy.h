#pragma once

#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <pwd.h>

#include "error.h"

#define MAX_DIR_SIZE 100
#define MAX_QUERY_SIZE 500
#define MAX_QUEIRES 1000
#define MAX_LENGTH 4096

typedef struct
{
    char* name;
    int arg_number;
    char** arguments;
    char* input_file;
    char* output_file;
    int output_type; //1 - переписать, 2 - добавить в конец
} program;

typedef struct
{
    int foreground;
    char* name; // Cтрока запроса 
    int number_of_programs;
    program* programs;
    int working;
    pid_t pid; // id
} job;


int leader_pid;
int current_id;

char curr_dir[MAX_DIR_SIZE];
char home_dir[MAX_DIR_SIZE];
job jobs[MAX_QUEIRES];
int jobs_number;


void initialize_program(program* prog, char* word);

void destroy_prog(program* prog);

void finish_work(int exit_flag);

void initialize_prog(char* query);

int set_environment_vars();