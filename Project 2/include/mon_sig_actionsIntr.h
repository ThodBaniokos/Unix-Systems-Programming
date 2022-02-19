// file: mon_sig_actionsIntr.h
// declaration of the signal handlers and actions of the monitor process
// include guard
#pragma once

// libs below
#include <stdbool.h>

// custom headers below
#include "monitorHelpersIntr.h"

// globals for signal handling in monitor process
extern bool received_sigusr1;
extern bool received_sigint;
extern bool received_sigquit;

// signal handlers for monitor process
void sigusr1_handler(int signal);
void sigint_handler(int signal);
void sigquit_handler(int signal);

// signal actions for monitor process
void readNewFiles(mon_con *container, char **input_dirs, int amount_of_sub_dirs, char **read_files, int amount_of_files, struct pollfd *files_descs, int buffer_size, int blm_size);
void produce_log_mon(mon_con container, req_stats *stats, char *log_path);