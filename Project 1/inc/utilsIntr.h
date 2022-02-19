#include <stdio.h>
#include "Types.h"

#pragma once

char *makeString(FILE **file_ptr);
char **stringParser(char *str, int *sizeMemAddress, char *separator);
void deleteParsedString(char **stringArray, int stringCounter);