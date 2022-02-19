// file: common.c
// contains common functions like a print or create
// function for ints/floats/doubles etc
// libraries below
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// custom headers below
#include "blmIntr.h"
#include "utilsIntr.h"
#include "commonIntr.h"
#include "hashFunctions.h"

// string related funcs
int compareStrings(Item str1, Item str2)
{
    return strcmp((char *)str1, (char *)str2);
}

// prints given string
void printString(Item str)
{
    printf("%s\n", (char *)str);
    return;
}

// creates a new string and copies the given one to it
char *createString(char *str)
{
    // allocate memory for the new string
    char *string = malloc((strlen(str) + 1) * sizeof(char));

    // check for correct memory allocation
    assert(string != null);

    // copy given string and return the new one
    strcpy(string, str);
    return string;
}

// general
void deletePointer(Item pointer)
{
    // try to deallocate memory only if the pointer points to something in the memory
    if (pointer) free(pointer);

    return;
}
