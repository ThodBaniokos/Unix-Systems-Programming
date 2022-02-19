// file: ioAPIIntr.h
// contains the declarations of the functions
// needed for the custom i/o to work
#pragma once

// custom headers below
#include "commandsIntr.h"
#include "travelMonitorClientHelpersIntr.h"

// macro defitions for custom exit code
#define EXIT -1

// void functions
void printPrompt(void);
void errorNotifier(const char **acceptableCommands, const int itemCounter);
void deleteBufferLine(char **line);

// int functions
int executeCommand(msi *proc_info, int numMonitors, struct pollfd *all_file_descs, int buf_size, char **input, int size, const char **commands, int numOfCommands, req_stats *stats, char *log_path);
int searchCountry(msi *proc_info, int numMonitors, const char *country);

bool checkValidCommands(const char **valid, const int size, char **input);

// char functions
char *getInput(void);
char **read_cmd(int *inputSize);
