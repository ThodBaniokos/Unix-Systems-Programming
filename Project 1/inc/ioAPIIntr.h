// file: ioAPIIntr.h
// contains the declarations of the functions
// needed for the custom i/o to work
#pragma once

#include "utilsIntr.h"
#include "vaccineMonitorHelpersIntr.h"

// void functions
void printPrompt(void);
void errorNotifier(const char **acceptableCommands, const int itemCounter);
void deleteBufferLine(char **line);

int executeCommand(vacData *container, char **input, int inputSize, const char **commands, int numOfCommands);

bool checkValidCommands(const char **valid, const int size, char **input);

// char * functions
char *getInput(void);
char **read_cmd(int *inputSize);
