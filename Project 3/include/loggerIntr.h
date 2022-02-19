// file: loggerIntr.h
// interface of the logger function used
// include guard
#pragma once

// libs below
#include <unistd.h>

// custom libs below
#include "Types.h"

// declaration of the logger function
void create_log(pid_t pid, char **countries, int amount_of_countries, req_stats *stats, char *path);