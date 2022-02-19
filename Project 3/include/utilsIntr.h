// file: utilsIntr.h
// declaration of the utility functions used through out the application
// include guard
#pragma once

// libs below
#include <stdio.h>

// custom headers below
#include "Types.h"

// char functions below
char *makeString(FILE **file_ptr);
char **stringParser(char *str, int *sizeMemAddress, char *separator);

// void functions below
void deleteParsedString(char **stringArray, int stringCounter);
