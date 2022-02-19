// file: commonIntr.h
// contains common functions declarations like a print or create
// function for ints/floats/doubles etc
#pragma once

// custom headers below
#include "Types.h"

// for chars
int compareStrings(Item str1, Item str2);
void printString(Item str);
char *createString(char *str);

// general
void deletePointer(Item pointer);
