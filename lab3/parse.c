#include "parse.h"


int skip_whitespaces_and_tabs(char* query, int current_pos) {
    while (query[current_pos] == ' ' || query[current_pos] == '\t')
        ++current_pos;
    return current_pos;
}

// Получает первое целое слово 
int get_word(char* string, int current, char** res) {
    int finish;
    int quotes = 0;
    *res = NULL;
    current = skip_whitespaces_and_tabs(string, current);
    if (string[current] == '\'') {
        quotes = 1;
    }
    if (string[current] == '\"') {
        quotes = 2;
    }
    for (finish = current; finish < (int)strlen(string) - 1; ++finish) {
        if (quotes == 1 && string[finish] != '\'') {
            continue;
        }
        if (quotes == 2 && string[finish] != '\"') {
            continue;
        }
        if (string[finish] == ' ') {
            break;
        }
    }
    if (quotes) {
        ++current;
        --finish;
    }
    *res = (char*)malloc((finish - current + 1) * sizeof(char));
    memcpy(*res, string + current, finish - current);
    (*res)[finish - current] = '\0';
    if (quotes) {
        return finish + 1;
    }
    return finish;
}

void parse(int start, char* source, program* prog) {
    char* word;
    start = get_word(source, start, &word);
    initialize_program(prog, word);
    free(word);
    while (1) {
        if (start >= (int)strlen(jobs[jobs_number].name) - 1) {
            break;
        }
        start = get_word(source, start, &word);
        if (!strcmp(word, "&") || !strcmp(word, "|") || word[0] == '|') {
            free(word);
            break;
        }
        if (!strcmp(word, "<")) {
            free(word);
            start = get_word(jobs[jobs_number].name, start, &word);
            prog->input_file = (char*)malloc(strlen(word) + 1);
            memcpy(prog->input_file, word, strlen(word) + 1);
            free(word);
            if (start >= (int)strlen(jobs[jobs_number].name) - 1) {
                break;
            }
        }
        if (!strcmp(word, ">")) {
            free(word);
            start = get_word(jobs[jobs_number].name, start, &word);
            prog->output_file = (char*)malloc(strlen(word) + 1);
            prog->output_type = 1;
            memcpy(prog->output_file, word, strlen(word) + 1);
            free(word);
            if (start >= (int)strlen(jobs[jobs_number].name) - 1) {
                break;
            }
        }
        if (!strcmp(word, ">>")) {
            free(word);
            start = get_word(jobs[jobs_number].name, start, &word);
            prog->output_file = (char*)malloc(strlen(word) + 1);
            prog->output_type = 2;
            memcpy(prog->output_file, word, strlen(word) + 1);
            free(word);
            if (start >= (int)strlen(jobs[jobs_number].name) - 1) {
                break;
            }
        }
        ++prog->arg_number;
        prog->arguments = (char**)realloc(prog->arguments, (prog->arg_number + 2) * sizeof(char*));
        prog->arguments[prog->arg_number] = (char*)malloc(strlen(word) + 1);
        memcpy(prog->arguments[prog->arg_number], word, strlen(word) + 1);
        prog->arguments[prog->arg_number + 1] = NULL;
        free(word);
    }
}