// file: serialization.c
// functions to serialize queries
// libs below
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// custom libs
#include "serializationIntr.h"

// macro defitions below
#define null NULL

// function below
char *serialize_query(char **parsed_query, int size)
{
    // length of the query
    int counter = 0;

    // calculate the query size
    for(int i = 0; i < size; i++)
    {
        int len = 0;
        if (i < size - 1) len = snprintf(NULL, len, "%s ", parsed_query[i]);
        else len = snprintf(NULL, len, "%s", parsed_query[i]);
        counter += len;
    }

    // allocate memory for the serialized query
    char *serialized = calloc(counter + 1, sizeof(char));

    // check for correct memory allocation
    assert(serialized != null);

    // copy the parsed query to the serialization string
    for (int i = 0; i < size; i++)
    {
        strcat(serialized, parsed_query[i]);
        if (i < size - 1) strcat(serialized, " ");
    }

    return serialized;
}
