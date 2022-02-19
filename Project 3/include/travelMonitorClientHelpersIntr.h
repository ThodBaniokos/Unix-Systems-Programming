// file: travelMonitorClientHelpersIntr.h
// interface of the travelMonitorClientHelpers.c implementations
// include guard
#pragma once

// libs below
#include <poll.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

// custom headers below
#include "Types.h"

// structs below
// struct of the information of each monitor process
typedef struct MonitorServerInfo
{
    // monitor process id
    pid_t mon_process_id;

    // port number
    int port_num;

    // client socket id to this process
    int c_socket_id;

    // sockaddr_in struct of the client
    struct sockaddr_in server_addr;

    // struct pollfd of the current process
    struct pollfd socketfd;

    // sub directories of each process
    char **subdirs;
    char **countries;
    int subdirsAmount;

    // bloom filters in each monitor server process
    int blooms_amount;
    blm *blooms;

    // boolean to know if the process is set or not
    bool isSet;
    bool isReady;

    // requests list
    HT travel_req_queries;

} msi;

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
int argc_argv_manipulator(int argc, char **argv, int *num_monitors, int *numThreads, int  *socket_buf_size, int *cyclic_buf_size, int *bloom_size, char **input_dir_path);
int create_named_pipes(const char *read_format, const char *write_format, int proc_amount, int subdirs);
int readDirectories(const char *input_dir_path, char ***fullpaths, int *inputDirs);
int compareDates(Item date1, Item date2);
int compareHtCon(Item con, Item cntr);

// void functions below
void general_initialization(void);
void process_initialization(msi *proc_info, int numMonitors, int numThreads, char *input_dir_path, char **full_paths, int subdirs, int s_buf_size, int c_buf_size, int init_port, int blm_size, struct pollfd *all_file_descs);
void destroyDates(Item item);
void printDates(Item item);
void printHtCon(Item to_print);
void destroyHtCon(Item discard);
void printMonitorProcessInfo(msi info[], int procs);
void deleteMonitorProcessInfo(msi discard[], int procs);
void copyNamedPipesPath(const char *write_fifo_path_name, const char *read_fifo_path_name, msi *proc_info, int index);
void distributeSubDirs(msi *proc_info, const char *input_dir_path, char **full_paths, int processes, int subdirs);
void setupMonitors(msi *proc_info, int numMonitors, int numThreads, int init_port, int s_buf_size, int c_buf_size, int blm_size);
void initPollfdStruct(msi *proc_info, int numMonitors, struct pollfd *all_file_descs, int all_file_descs_size);
void reinitializeMonitor(msi *proc_info, int numMonitors);
void sendBufferAndBloomSize(msi *proc_info, int numMonitors, int buf_size, int blm_size, struct pollfd *all_file_descs);
void sendSubDirsAmount(msi *proc_info, int numMonitors, int buf_size, struct pollfd *all_file_descs);
void sendSubDirs(msi *proc_info, int numMonitor, int buf_size, struct pollfd *all_file_descs);
void receiveBlooms(msi *proc_info, int numMonitors, struct pollfd *all_file_descs, int all_file_descs_size, int buf_size);
void readinessCheck(msi *proc_info, int numMonitors, int buf_size, struct pollfd *all_file_descs, int all_file_descs_size);

// char functions below
char **buildArgs(char *programName, msi proc_info, int numThreads, int s_buf_size, int c_buf_size, int blm_size, int *size);