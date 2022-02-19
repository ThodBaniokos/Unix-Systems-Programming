// file: travelMonitorHelpersIntr.h
// interface of the travelMonitorHelpers.c implementations
// include guard
#pragma once

// libs below
#include <stdbool.h>
#include <stdlib.h>
#include <poll.h>

// custom headers below
#include "Types.h"

// structs below
// struct of the information of each monitor process
typedef struct MonitorProcesInfo
{
    // monitor process id
    pid_t mon_process_id;

    // read related path and fd
    char *read_np;
    int read_fd;

    // write related path and fd
    char *write_np;
    int write_fd;

    // input subdirectories and amount
    char **subdirs;
    char **countries;
    int subdirsAmount;

    // poll struct
    struct pollfd file_descs[FILE_DESCRIPTORS];

    // bloom filters of this child process
    int blooms_amount;
    blm *blooms;

    // requests list
    HT travel_req_queries;

    // boolean to know if the process is set or not
    bool isSet;

} mpi;

// queries information
typedef struct request_information
{
    char *date;
    int accepted;
    int rejected;
} *req_info;

// used to store the given queries
typedef struct hash_container
{
    char *country;
    lList viruses;
} *ht_con;

// bool functions below
bool isNumber(char *str, int len);

// int functions below
int argc_argv_manipulator(int argc, char **argv, int *num_monitors, int  *buf_size, int *bloom_size, char **input_dir_path);
int create_named_pipes(const char *read_format, const char *write_format, int proc_amount, int subdirs);
int readDirectories(const char *input_dir_path, char ***fullpaths, int *inputDirs);
int compareDates(Item date1, Item date2);
int compareHtCon(Item con, Item cntr);

// void functions below
void general_initialization(void);
void process_initialization(mpi *proc_info, int numMonitors, char *input_dir_path, char **full_paths, int subdirs, char *read_fifo_path, char *write_fifo_path, int buf_size, int blm_size, struct pollfd *all_file_descs);
void destroyDates(Item item);
void printDates(Item item);
void printHtCon(Item to_print);
void destroyHtCon(Item discard);
void printMonitorProcessInfo(mpi info[], int procs);
void deleteMonitorProcessInfo(mpi discard[], int procs);
void copyNamedPipesPath(const char *write_fifo_path_name, const char *read_fifo_path_name, mpi *proc_info, int index);
void distributeSubDirs(mpi *proc_info, const char *input_dir_path, char **full_paths, int processes, int subdirs);
void setupMonitors(mpi *proc_info, int numMonitors, char *read_fifo_path_name, char *write_fifo_path_name);
void initPollfdStruct(mpi *proc_info, int numMonitors, struct pollfd *all_file_descs, int all_file_descs_size);
void reinitializeMonitor(mpi *proc_info, int numMonitors);
void sendBufferAndBloomSize(mpi *proc_info, int numMonitors, int buf_size, int blm_size, struct pollfd *all_file_descs);
void sendSubDirsAmount(mpi *proc_info, int numMonitors, int buf_size, struct pollfd *all_file_descs);
void sendSubDirs(mpi *proc_info, int numMonitor, int buf_size, struct pollfd *all_file_descs);
void receiveBlooms(mpi *proc_info, int numMonitors, struct pollfd *all_file_descs, int all_file_descs_size, int buf_size);
