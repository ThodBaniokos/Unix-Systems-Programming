// file: ioAPI.c
// contains the implementations of the functions
// needed by the i/o api
// libraries below
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "commandsIntr.h"
#include "commonIntr.h"
#include "ioAPIIntr.h"

// macro defitions used to determine commands
#define vacStatusBloom 1
#define vacStatus 2
#define popStatus 3
#define popStatusAge 4
#define insertCitizen 5
#define vacNow 6
#define listNonVac 7
#define _exit 8

// void functions
void printPrompt(void) { printf("vaccineMonitor:~$ "); }

void errorNotifier(const char **acceptableComands, const int count)
{
    printf("ERROR IN TYPED COMMAND\nACCEPTABLE COMMANDS, AGRUMENTS IN [] CAN BE OMMITED\n");
    for(int i = 0; i < count; i++)
    {
        switch (i + 1)
        {
            case vacStatusBloom:
                printf("%s citizenID virusName\n", acceptableComands[i]);
                break;
            case vacStatus:
                printf("%s citizenID [virusName]\n", acceptableComands[i]);
                break;
            case popStatus:
                printf("%s [country] virusName date1 date2\n", acceptableComands[i]);
                break;
            case popStatusAge:
                printf("%s [country] virusName date1 date2\n", acceptableComands[i]);
                break;
            case insertCitizen:
                printf("%s citizenID firstName lastName country age virusName YES/NO [date]\n", acceptableComands[i]);
                break;
            case vacNow:
                printf("%s citizenID firstName lastName country age virsuName\n", acceptableComands[i]);
                break;
            case listNonVac:
                printf("%s virusName\n", acceptableComands[i]);
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
int executeCommand(vacData *container, char **input, int size, const char **commands, int numOfCommands)
{
    // vaccineMonitor commands format, arguments in [] may not be given
    // /vaccineStatusBloom citizenId virusName
    // /vaccineStatus citizenId virusName or citizenId
    // /populationStatus [country] virusName date1 date2
    // /popStatusByAge [country] virusName date1 date2
    // /insertCitizenRecord citizenID firstName lastName country age virusName YES/NO [date] 
    // /vaccinateNow citizenID firstName lastName country age virusName
    // /list-nonVaccinated-Persons virusName
    // /exit

    // check which command is given
    for(int i = 0; i < numOfCommands; i++)
    {
        if(!strcmp(input[0], commands[i]))
        {
            switch (i + 1)
            {
                case vacStatusBloom:
                    vaccineStatusBloomSequence(container, input[1], input[2]);
                    deleteParsedString(input, size);
                    return SUCCESS;
                case vacStatus:
                    if (size > 2) vaccineStatus(container, input[1], input[2]);
                    else vaccineStatus(container, input[1], null);
                    deleteParsedString(input, size);
                    return SUCCESS;
                case popStatus:
                    if (size > 4) popStatusSequence(container, input[1], input[2], input[3], input[4]);
                    else popStatusSequence(container, null, input[1], input[2], input[3]);
                    deleteParsedString(input, size);
                    return SUCCESS;
                case popStatusAge:
                    if (size > 4) popStatusByAgeSequence(container, input[1], input[2], input[3], input[4]);
                    else popStatusByAgeSequence(container, null, input[1], input[2], input[3]);
                    deleteParsedString(input, size);
                    return SUCCESS;
                case insertCitizen:
                    if (size > 8) insertCitizenSequence(container, input[1], input[2], input[3], input[4], atoi(input[5]), input[6], input[7], input[8]);
                    else insertCitizenSequence(container, input[1], input[2], input[3], input[4], atoi(input[5]), input[6], input[7], null);
                    deleteParsedString(input, size);
                    return SUCCESS;
                case vacNow:
                    vaccinateNowSequence(container, input[1], input[2], input[3], input[4], atoi(input[5]), input[6]);
                    deleteParsedString(input, size);
                    return SUCCESS;
                case listNonVac:
                    listNonVacPersonsSequence(container, input[1]);
                    deleteParsedString(input, size);
                    return SUCCESS;
                case _exit:
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

// char functions
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