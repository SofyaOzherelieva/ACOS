#pragma once

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "format_mes.h"

typedef struct CSomeArgs {
    CServerMood* current;
    int* id;
    int thread_id;
} CSomeArgs;


void* processConnection(void* args);