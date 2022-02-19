// file: travelMonitor.c
// implementation of the main process of the program
// libraries below
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

// custom headers below
#include "ioAPIIntr.h"
#include "utilsIntr.h"
#include "commonIntr.h"
#include "commandsIntr.h"
#include "sig_actionsIntr.h"
#include "ipc_protocolIntr.h"
#include "travelMonitorHelpersIntr.h"

// path of the logs directory
#define LOGSPATH "./logs/"

// number of commands and acceptable commands
const int numOfCommands = 5;

const char *commands[] = {
    "/travelRequest",
    "/travelStats",
    "/addVaccinationRecords",
    "/searchVaccinationStatus",
    "/exit"
};

// path and name format for each named pipe
char *read_fifo_path_name = "./tmp/main_proc_write_";
char *write_fifo_path_name = "./tmp/child_proc_write_";

int main(int argc, char *argv[])
{
    // init signal handlers and ipc protocol variables
    general_initialization();

    // number of monitor proccesses and bloom filter size
    // and size of buffer that will be used for the named pipes
    int numMonitors, blm_size, buf_size;

    // input directory path
    char *input_dir_path;

    // full paths of the input directories and amount of subdirs
    char **full_paths;
    int subdirs;

    // try to read the command line arguments and if an error occurs stop the execution
    if (argc_argv_manipulator(argc, argv, &numMonitors, &buf_size, &blm_size, &input_dir_path))
    {
        perror("ERROR READING COMMAND LINE ARGUMENTS");
        exit(EXIT_FAILURE);
    }

    // reading all the contents of the input directory alphabeticaly
    // and building the fullpaths of the input sub directories
    if (readDirectories(input_dir_path, &full_paths, &subdirs) == EXIT_FAILURE)
    {
        perror("ERROR READING SUB DIRECTORIES");
        exit(EXIT_FAILURE);
    }

    // creating named pipes for ipc before the creation of the child processes
    if (create_named_pipes(read_fifo_path_name, write_fifo_path_name, numMonitors, subdirs))
    {
        perror("ERROR CREATING FIFO's");
        exit(EXIT_FAILURE);
    }

    // checking if the subdirectories are less than the processes tbc
    // if that is true there's no point in creating monitors with no
    // countries to manage, create as many monitors as the sudirs
    if (numMonitors > subdirs) numMonitors = subdirs;


    // create an array of the needed information of every process
    mpi proc_info[numMonitors];

    // create a struct to store all the file descriptors for polling
    struct pollfd all_file_descs[numMonitors * FILE_DESCRIPTORS];

    // create and initialize all processes
    process_initialization(proc_info, numMonitors, input_dir_path, full_paths, subdirs, read_fifo_path_name, write_fifo_path_name, buf_size, blm_size, all_file_descs);

    // delete allocated memory for full_paths if exists
    if (full_paths) free(full_paths);

    // ready to send queries
    bool isRunning = true;
    char **input;
    int inputSize, returnVal;
    req_stats stats;
    stats.accepted = stats.rejected = stats.total = 0;

    // receiving queries
    while (isRunning)
    {

        // check if a sigint or a sigquit signal was caught
        if (received_sigint || received_sigquit)
        {
            produce_log(proc_info, numMonitors, &stats, LOGSPATH, &isRunning);
            continue;
        }

        // read command
        input = read_cmd(&inputSize);

        // check if a child process died and create a new one, because the monitor process that dies might be
        // the one that we need to find the correct answer to the given query
        if (received_sigchld)
        {
            recreate_monitor(proc_info, numMonitors, all_file_descs, buf_size, blm_size);
        }

        // check if input was given
        if (!input)
        {
            printf("ERROR: NO INPUT GIVEN\n");
            continue;
        }

        // check if the command given is a valid one
        if (!checkValidCommands(commands, numOfCommands, input))
        {
            errorNotifier(commands, numOfCommands);
            deleteParsedString(input, inputSize);
            continue;
        }

        // excecute the command given
        if ((returnVal = executeCommand(proc_info, numMonitors, all_file_descs, buf_size, input, inputSize, commands, numOfCommands, &stats, LOGSPATH)) == EXIT_FAILURE)
        {
            perror("execute command at travelMonitor.c");
            exit(EXIT_FAILURE);
        }
        else if (returnVal == EXIT)
        {
            deleteMonitorProcessInfo(proc_info, numMonitors);
            isRunning = false;
            continue;
        }
    }

    exit(EXIT_SUCCESS);
}