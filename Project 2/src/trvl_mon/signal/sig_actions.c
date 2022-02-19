// file: sig_actions.c
// implementation of the signal related functions
// libs below
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>

// custom headers below
#include "commandsIntr.h"
#include "sig_actionsIntr.h"

// globals below
bool received_sigint;
bool received_sigquit;
bool received_sigchld;

// signal handling functions
// SIGINT
void sigint_handler(int signal)
{
    if (signal == SIGINT) received_sigint = true;
    else return;
}

// SIGQUIT
void sigquit_handler(int signal)
{
    if (signal == SIGQUIT) received_sigquit = true;
    else return;
}

// SIGCHLD
void sigchld_handler(int signal)
{
    if (signal == SIGCHLD) received_sigchld = true;
    else return;
}

// signal actions functions
void produce_log(mpi *proc_info, int numMonitors, req_stats *stats, char *log_path, bool *isRunning)
{
    if (!exit_sequence_query(proc_info, numMonitors, stats, log_path))
    {
        deleteMonitorProcessInfo(proc_info, numMonitors);
        *isRunning = false;
        return;
    }
    exit(EXIT_FAILURE);
}

void recreate_monitor(mpi *proc_info, int numMonitors, struct pollfd *all_file_descs, int buf_size, int blm_size)
{
    pid_t dead;
    int status;

    while ((dead = waitpid(-1, &status, WNOHANG)) > 0)
    {
        for(int i = 0; i < numMonitors; i++)
        {
            if (proc_info[i].mon_process_id == dead)
            {
                proc_info[i].mon_process_id = -1;
                break;
            }
        }
    }

    for(int i = 0; i < numMonitors; i++) reinitializeMonitor(proc_info, numMonitors);

    // reinitialize the pollfd structs with the new file descriptors
    initPollfdStruct(proc_info, numMonitors, all_file_descs, numMonitors * FILE_DESCRIPTORS);

    // first we're sending the buffer size, and the bloom filter size
    sendBufferAndBloomSize(proc_info, numMonitors, buf_size, blm_size, all_file_descs);

    // sending amount of subdirs of each monitor
    sendSubDirsAmount(proc_info, numMonitors, buf_size, all_file_descs);

    // sending the subdirs
    sendSubDirs(proc_info, numMonitors, buf_size, all_file_descs);

    // receiving the bloom filters
    receiveBlooms(proc_info, numMonitors, all_file_descs, numMonitors * FILE_DESCRIPTORS, buf_size);

    // reset signal handler flag
    received_sigchld = false;
}
