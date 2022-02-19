// file: vaccineMonitor.c
// main function of the application
// c libs
#include <stdio.h>
#include <stdlib.h>

// custom libs
#include "ioAPIIntr.h"
#include "commandsIntr.h"
#include "vaccineMonitorHelpersIntr.h"

// amount of commands
static const int numOfCommands = 8;

// array with acceptable commands for the app
const char *commands[] = {
    "/vaccineStatusBloom",
    "/vaccineStatus",
    "/populationStatus",
    "/popStatusByAge",
    "/insertCitizenRecord",
    "/vaccinateNow",
    "/list-nonVaccinated-Persons",
    "/exit"
};

int main(int argc, char *argv[])
{
    // input file stream
    FILE *inputStream = null;

    // bloom size in bytes, size of user typed input, return value if executeCommand
    int bloom_size, inputSize, returnVal;

    // bool to determine if the program is running or not
    bool isRunning = true;

    // user input from command line
    char **input;

    // vaccine monitor data container
    vacData container = malloc(sizeof(*container));

    // read the arguments from the command line and if an error occurs abort execution of the app
    if (argc_argv_manipulator(argc, argv, &inputStream, &bloom_size) == ERROR)
    {
        printf("ERROR: WRONG ARGUMENTS GIVEN\nUSAGE: ./vaccineMonitor -c citizenRecordsFile -b bloomSize\n");
        return ERROR;
    }

    // set the bloom size in the container
    container->bloom_size = bloom_size;

    // read the input stream, if there's no file abort execution
    if (inputStream != null) initDataStructs(&container, &inputStream);
    else 
    {
        printf("ERROR: NO INPUT FILE GIVEN\n");
        return ERROR;
    }

    // starting the application
    while (isRunning)
    {
        // get user input
        input = read_cmd(&inputSize);

        // check if input is empty
        if(!input)
        {
            printf("ERROR IN TYPED INPUT, PLEADE RETRY.\n");
            continue;
        }

        // check if the given input is an acceptable command
        if(!checkValidCommands(commands, numOfCommands, input))
        {
            // notify the user of the error in the typed command
            errorNotifier(commands, numOfCommands);
            deleteParsedString(input, inputSize);
            continue;
        }

        // try to execute given command, if there's an error notify user and stop program execution
        // if return value is EXIT the deallocate memory and normaly stop the program execution
        if((returnVal = executeCommand(&container, input, inputSize, commands, numOfCommands)) == ERROR)
        {
            printf("ERROR ENCOUNTERED\n");
            return ERROR;
        }
        else if (returnVal == EXIT)
        {
            exitSequence(&container, &inputStream);
            isRunning = false;
            continue;
        }
    }

    return EXIT;
}