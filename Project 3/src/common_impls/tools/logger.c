// file: logger.c
// implementation of the logger function used
// libs below
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// custom libs below
#include "loggerIntr.h"

// create a log file
void create_log(pid_t pid, char **countries, int amount_of_countries, req_stats *stats, char *path)
{
    // get the length of chars needed for the process id creating this log file
    int len = 0;
    len = snprintf(null, len, "%u", pid);

    // convert process id
    char pidStr[len + 1];
    snprintf(pidStr, len + 1, "%u", pid);

    // create the file path of the log file
    char file_path[strlen(path) + strlen("log_file.") + len + 1];

    // set every char to null since there's nothing, avoiding garbage values
    memset(file_path, '\0', strlen(path) + 1 + strlen("log_file.") + len + 1);

    // copy the needed fields to create the log file
    strcpy(file_path, path);
    strcat(file_path, "log_file.");
    strcat(file_path, pidStr);

    // create the file with fopen and write mode
    FILE *log_file = fopen(file_path, "w");

    // copy countries monitored by the process
    for(int i = 0; i < amount_of_countries; i++)
    {
        fprintf(log_file, "%s\n", countries[i]);
    }

    // copy the statistics stored in the given req_stats struct
    fprintf(log_file, "TOTAL TRAVEL REQUESTS %d\n", stats->total);
    fprintf(log_file, "ACCEPTED %d\n", stats->accepted);
    fprintf(log_file, "REJECTED %d\n", stats->rejected);

    // close the file
    fclose(log_file);

    return;
}
