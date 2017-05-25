#pragma once

#include <errno.h>
#include <stdio.h>

#define TOO_MANY_ARGUMENTS 1
#define NO_SUCH_DIRECTORY 2
#define ENV_VARS_FAIL 3
#define CD_FAIL 4

void my_perror(int error);