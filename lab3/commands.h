#pragma once

#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <dirent.h>
#include "init_destroy.h"


int write_history(program* prog);

int cd_execute(program* prog);

void change_dir(const char* mod);

void change_dir_simple(const char* mod);