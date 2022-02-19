// file: vaccineMonitorHelpersIntr.h
// declaration of structs, functions, etc. that are going to help us build
// the needed vaccineMonitor behaviours for the correct excecution of the app
#pragma once

// custom libs
#include "hashFunctions.h"
#include "hashTableIntr.h"
#include "blmIntr.h"
#include "skipListIntr.h"
#include "commonIntr.h"
#include "linkedListIntr.h"
#include "personIntr.h"
#include "utilsIntr.h"

// stucts below
// this is going to be the container of the bloom filters, skip lists and persons for this app
typedef struct vaccinationData
{
    HT persons;
    HT bloom_filters;
    HT skip_list_vaccinated;
    HT skip_list_not_vaccinated;
    lList countries;
    lList viruses;
    int bloom_size;
} *vacData;

// this is going to represent the vaccination status of a person for a certain virus
typedef struct personVaccination
{
    pInfo person;
    bool isVaccinated;
    char *date;
} *personVac;

// this is going to store the country information, i.e. population and population by age group
typedef struct countryInformation
{
    char *country;
    int population;
    int till20;
    int till40;
    int till60;
    int olderThan60;
} *countryInfo;

// functions below
// personVac
// creates a person vaccination instance of the struct
personVac createPersonVaccinationData(pInfo person, bool isVaccinated, char *date);

// int
// compares the struct with the given id
int comparePersonVaccinationData(Item data, Item id);

// reads the argc and argv given from the command line
int argc_argv_manipulator(int argc, char **argv, FILE **inputStream, int *bloom_size);

// compares the countries given
int compareCountry(Item countryInfo, Item country);

// checks if the person info are the same with the given file input
int comparePersonInfo(Item person, char *firstName, char *lastName, char *country, unsigned short int age);

// void
// prints person vaccination data
void printPersonVaccinationData(Item data);

// destroys the vaccination data of the main program
void destroyVacData(vacData dicsard);

// destroys the person vaccination data
void destroyPersonVaccination(Item discard);

// destroys the country data in the country information struct
void destroyCountryInfo(countryInfo discard);

// opens input stream given in command line
void openIStream(FILE **stream, char *path);

// initializes data structs with the given input file
void initDataStructs(vacData *container, FILE **stream);