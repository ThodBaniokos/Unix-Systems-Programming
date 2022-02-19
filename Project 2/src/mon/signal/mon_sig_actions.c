// file: mon_sig_actions.c
// implementation of the signal related functions of the monitor process
// libs below
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

// custom headers below
#include "loggerIntr.h"
#include "mon_commandsIntr.h"
#include "mon_sig_actionsIntr.h"

// globals for signal handling
bool received_sigusr1 = false;
bool received_sigint = false;
bool received_sigquit = false;

// signal handlers for monitor process
void sigusr1_handler(int signal)
{
    if (signal == SIGUSR1) received_sigusr1 = true;
    else return;
}

void sigint_handler(int signal)
{
    if (signal == SIGINT) received_sigint = true;
    else return;
}

void sigquit_handler(int signal)
{
    if (signal == SIGQUIT) received_sigquit = true;
    else return;
}

// signal actions functions for monitor process
// read new files and updates data structs
void readNewFiles(mon_con *container, char **input_dirs, int amount_of_sub_dirs, char **read_files, int amount_of_files, struct pollfd *files_descs, int buffer_size, int blm_size)
{
    // update the data structs since we received a sigusr1 signal
    if (updateDataStructs(container, input_dirs, amount_of_sub_dirs, read_files, amount_of_files) == EXIT_FAILURE) exit(EXIT_FAILURE);

    // send the updated bloom filters to the main process
    sendBlooms(container, files_descs, buffer_size, blm_size);

    // revert the flag of the signal
    received_sigusr1 = false;

    return;
}

// create a log file and place it in the logs/ directory
void produce_log_mon(mon_con container, req_stats *stats, char *log_path)
{
    // get the countries monitored by this process
    int size = getListSize(container->countries);
    char **countries = calloc(size, sizeof(char *));

    // check for correct memory allocation
    assert(countries != null);

    // set the countries, copy the pointers since we're only allocating memory to temporalily store
    // them to an array used in create_log
    for(int i = 0; i < size; i++)
    {
        countries[i] = getDataNode(getListNodeAt(container->countries, i));
    }

    // create log of the process with given id, countries monitored, size and store it to log_path
    create_log(getpid(), countries, size, stats, log_path);

    // free allocated memory of the countries array if exists
    if (countries) free(countries);

    // revert sigint flag since we've received a signal
    // this function is called when a sigquit is caught
    // but if the sigquit is caught then the process is
    // killed, since this is the behaviour of sigquit,
    // a new one replaces this process
    if (received_sigint) received_sigint = false;
}
