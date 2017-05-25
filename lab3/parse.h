#pragma once

#include <stdlib.h>
#include <string.h>
#include "init_destroy.h"

void parse(int start, char* source, program* prog);

int get_word(char* string, int current, char** res);

int skip_whitespaces_and_tabs(char* query, int current_pos);