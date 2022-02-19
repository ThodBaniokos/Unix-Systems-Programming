// file: errorChecking.c
// implementation of the error checking functions in the travel monitor application
// libs below
#include <stdio.h>

// custom headers below
#include "errorCheckingIntr.h"

// checks if given string is composed with digits
bool isNumber(char *str, int len)
{
    for(int i = 0; i < len; i++)
    {
        // if the character in the ith place is a dash or digit
        // if not return false, else return true
        if(str[i] == '-' || (str[i] >= '0' && str[i] <= '9')) continue;
        else return false;
    }

    return true;
}
