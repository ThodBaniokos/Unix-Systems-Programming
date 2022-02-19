// file: personIntr.h
// decleration and some util funcs for the struct of person
#pragma once

// libs
#include <stdbool.h>

// custom libs
#include "Types.h"

// struct for persons
typedef struct PersonInformation
{
    char *citizenID;
    char *firstName;
    char *lastName;
    char *country;
    short int age;
} *pInfo;

// pInfo
pInfo createPerson(char *id, char *firstName, char *lastName, char *country, unsigned short int age);

// char
char *getCitizenId(pInfo person);
char *getFirstName(pInfo person);
char *getLastName(pInfo person);
char *getCountry(pInfo person);

// unsigned short int
unsigned short int getAge(pInfo person);

// int
int compareCitizenId(Item citizen, Item id);

// void
void printPersonInformation(Item person);
void deletePerson(Item person);