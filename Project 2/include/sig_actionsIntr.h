// file: sig_actionsIntr.h
// declaration of the signal handlers and actions
// include guard
#pragma once

// libs below
#include <stdbool.h>

// custom headers below
#include "travelMonitorHelpersIntr.h"

// signal handling global booleans
extern bool received_sigint;
extern bool received_sigquit;
extern bool received_sigchld;

// signal handling functions below
void sigint_handler(int signal);
void sigquit_handler(int signal);
void sigchld_handler(int signal);

// signal actions functions below
void produce_log(mpi *proc_info, int numMonitors, req_stats *stats, char *log_path, bool *isRunning);
void recreate_monitor(mpi *proc_info, int numMonitors, struct pollfd *all_file_descs, int buf_size, int blm_size);