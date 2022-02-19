// file: ioAPI.c
// contains the implementations of the functions
// needed by the i/o api
// libraries below
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// custom headers below
#include "blmIntr.h"
#include "ioAPIIntr.h"
#include "utilsIntr.h"
#include "commonIntr.h"
#include "ipc_protocolIntr.h"

// macro defitions used to determine commands
#define travelRequest 1
#define travelStats 2
#define addVacRecs 3
#define searchVacRecs 4
#define _exit 5

// void functions
void printPrompt(void) { printf("travelMonitorClient:~$ "); }

void errorNotifier(const char **acceptableComands, const int count)
{
    printf("ERROR IN TYPED COMMAND\nACCEPTABLE COMMANDS, AGRUMENTS IN [] CAN BE OMMITED\n");
    for(int i = 0; i < count; i++)
    {
        switch (i + 1)
        {
            case travelRequest:
                printf("%s citizenID date countryFrom countryTo virusName\n", acceptableComands[i]);
                break;
            case travelStats:
                printf("%s virusName date1 date2 [country]\n", acceptableComands[i]);
                break;
            case addVacRecs:
                printf("%s country\n", acceptableComands[i]);
                break;
            case searchVacRecs:
                printf("%s citizenID\n", acceptableComands[i]);
                break;
            case _exit:
                printf("%s\n", acceptableComands[i]);
                break;
        }
    }
    return;
}

void deleteBufferLine(char **line) { free(*line); return; }

// int functions
int executeCommand(msi *proc_info, int numMonitors, struct pollfd *all_file_descs, int buf_size, char **input, int size, const char **commands, int numOfCommands, req_stats *stats, char *log_path)
{
    // travelMonitor commands input format
    // /travelRequest citizenID date countryFrom countryTo virusName

    // check which command is given
    for(int i = 0; i < numOfCommands; i++)
    {
        if(!strcmp(input[0], commands[i]))
        {
            switch (i + 1)
            {
                case travelRequest:
                    if (size != 6)
                    {
                        printf("ERROR: WRONG ARGUMENTS GIVEN\nACCEPTED FORMAT : /travelRequest citizenID date countryFrom countryTo virusName\n");
                        deleteParsedString(input, size);
                        return EXIT_SUCCESS;
                    }
                    if (trvl_request_sequence(proc_info, numMonitors, input, size, buf_size, stats, all_file_descs, numMonitors) == EXIT_FAILURE) return EXIT_FAILURE;
                    deleteParsedString(input, size);
                    return EXIT_SUCCESS;
                case travelStats:
                    if (size > 5 || size < 4)
                    {
                        printf("ERROR: WRONG ARGUMENTS GIVEN\nACCEPTED FORMAT : /travelStats virusName date1 date2 [country]\n");
                        deleteParsedString(input, size);
                        return EXIT_SUCCESS;
                    }
                    if (trvl_stats_sequence(proc_info, numMonitors, all_file_descs, numMonitors, input, size, buf_size) == EXIT_FAILURE) return EXIT_FAILURE;
                    deleteParsedString(input, size);
                    return EXIT_SUCCESS;
                case addVacRecs:
                    if (size > 2)
                    {
                        printf("ERROR: WRONG ARGUMENTS GIVEN\nACCEPTED FORMAT : /addVaccinationRecords country\n");
                        deleteParsedString(input, size);
                        return EXIT_SUCCESS;
                    }
                    if (add_vac_recs_sequence(proc_info, numMonitors, input, size, buf_size, all_file_descs, numMonitors) == EXIT_FAILURE) return EXIT_FAILURE;
                    deleteParsedString(input, size);
                    return EXIT_SUCCESS;
                case searchVacRecs:
                    if (size > 2)
                    {
                        printf("ERROR: WRONG ARGUMENTS GIVEN\nACCEPTED FORMAT : /searchVaccinationStatus citizenID\n");
                        deleteParsedString(input, size);
                        return EXIT_SUCCESS;
                    }
                    if (search_vac_status_sequence(all_file_descs, numMonitors, numMonitors, input, size, buf_size) == EXIT_FAILURE) return EXIT_FAILURE;
                    deleteParsedString(input, size);
                    return EXIT_SUCCESS;
                case _exit:
                    if (exit_sequence_query(proc_info, numMonitors, stats, log_path, all_file_descs, numMonitors * 2, buf_size) == EXIT_FAILURE) return EXIT_FAILURE;
                    deleteParsedString(input, size);
                    return EXIT;
            }
        }
    }

    return 0;
}

// bool functions
bool checkValidCommands(const char **validCommands, const int size, char **input)
{
    bool isValid = false;
    for(int i = 0; i < size; i++)
    {
        // check the given input with every valid command to see if we can execute it
        if(!strcmp(validCommands[i], input[0])) isValid = true;
    }
    return isValid;
}

// gets user input
char *getInput(void)
{
    char *input;
    printPrompt();
    input = makeString(&stdin);
    return input;
}

// reads the command given by the user
char **read_cmd(int *inputSize)
{
    char **input, *str;
    str = getInput();
    input = stringParser(str, inputSize, " ");
    deleteBufferLine(&str);
    return input;
}
