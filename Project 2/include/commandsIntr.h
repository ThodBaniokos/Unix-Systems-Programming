// file: commandsIntr.h
#pragma once

// custom headers below
#include "travelMonitorHelpersIntr.h"

// commands of the travel monitor app, executed in monitor app, travel monitor app gets the answer only
int trvl_request_sequence(mpi *proc_info, int numMonitors, char **input, int inputSize, int buf_size, req_stats *stats);
int trvl_stats_sequence(mpi *proc_info, int numMonitors, struct pollfd *all_file_descs, int pollfd_size, char **input, int inputSize, int buf_size);
int add_vac_recs_sequence(mpi *proc_info, int numMonitors, char **input, int inputSize, int buf_size);
int search_vac_status_sequence(struct pollfd *all_file_descs, int numMonitors, int pollfd_size, char **input, int input_size, int buf_size);
int exit_sequence_query(mpi *proc_info, int numMonitors, req_stats *stats, char *log_path);
