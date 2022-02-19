// file: commonIntr.h
// contains common functions declarations like a print or create
// function for ints/floats/doubles etc
#pragma once

#include "Types.h"

// for ints
void printInt(Item data);

int compareInt(Item num1, Item num2);

int qsortCmp(const void *num1, const void* num2);

void *createInt(int num);

// for floats
void printFloat(Item data);

float compareFloat(Item num1, Item num2);

void *createFloat(float num);

// for chars
int compareStrings(Item str1, Item str2);
void printString(Item str);
char *createString(char *str);

// general
void deletePointer(Item pointer);