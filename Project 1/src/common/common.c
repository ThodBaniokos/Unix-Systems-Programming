// file: common.c
// contains common functions like a print or create
// function for ints/floats/doubles etc
// libraries below
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// custom headers below
#include "commonIntr.h"

// string related funcs
int compareStrings(Item str1, Item str2)
{
    return strcmp((char *)str1, (char *)str2);
}

void printString(Item str)
{
    printf("%s\n", (char *)str);
    return;
}

char *createString(char *str)
{
    char *string = malloc((strlen(str) + 1) * sizeof(char));
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
