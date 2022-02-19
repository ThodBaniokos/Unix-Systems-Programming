// file: utils.c
// utility function implementations
// libs below
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// custom headers below
#include "utilsIntr.h"

char *makeString(FILE **file)
{
    // init variables
    char c, *str;
    int count = 1;

    // allocate memory for returning string
    str = malloc(count * sizeof(char));
    assert(str != null);
    str[count - 1] = '\0';

    // read char by char the file and convert it to a single string
    while ((c = fgetc(*file)) != EOF && c != '\n')
    {
        // if the char is not EOF or new line allocate memory for a new string bigger by one character
        count++;
        char *temp = malloc(count * sizeof(char));
        assert(temp != null);

        // copy contents
        strcpy(temp, str);

        // deallocate memory use before
        free(str);

        // add the new char and the end string character \0
        temp[count - 2] = c;
        temp[count - 1] = '\0';

        // point the return pointer to the returning variable
        str = temp;
    }

    // return the new string
    return str;
}

char **stringParser(char *str, int *size, char *sep)
{
    // catch possilbe input errors
    if(!(int)strlen(str)) return null;

    // init variables and memory allocation
    int count = 1;
    char **res, *token = strtok(str, sep);
    res = malloc(count * sizeof(char *));

    // check for correct memory allocation
    assert(res != null);

    // allocate and check memory for the initialization string
    res[count - 1] = malloc((int)strlen(token) + 1);
    assert(res[count - 1] != null);
    strcpy(res[count - 1], token);
    res[count - 1][strlen(token)] = '\0';

    // create the parsed string from the given one
    while (token != null)
    {
        // get new token
        token = strtok(null, sep);
        if(token == null) break;

        // reallocate memory with malloc, because we need to hve the same char size
        count++;
        char **temp = malloc(count * sizeof(char *));
        assert(temp != null);
        for(int i = 0; i < count - 1; i++)
        {
            // allocate memory for every new cell copy content from old one and delete allocated memory
            temp[i] = malloc(((int)strlen(res[i]) + 1) * sizeof(char));
            assert(temp[i] != null);
            strcpy(temp[i], res[i]);
            temp[i][strlen(res[i])] = '\0';
            free(res[i]);
        }
        free(res);
        temp[count - 1] = malloc(((int)strlen(token) + 1) * sizeof(char));
        assert(temp[count - 1] != null);
        strcpy(temp[count - 1], token);
        temp[count - 1][strlen(token)] = '\0';

        // make the pointer point to the return variable
        res = temp;
    }

    // return values
    *size = count;
    return res;
}

// delete memory allocate for parsed string
void deleteParsedString(char **strArray, int stringCounter)
{
    for(int i = 0; i < stringCounter; i++) free(strArray[i]);
    free(strArray);
    return;
}