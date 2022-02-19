// file: monitorHelpersIntr.h
// interface of the monitor helper functions
// include guard
#pragma once

// libs
#include <poll.h>

// custom libs below
#include "Types.h"
#include "blmIntr.h"
#include "utilsIntr.h"
#include "personIntr.h"
#include "commonIntr.h"
#include "skipListIntr.h"
#include "hashTableIntr.h"
#include "hashFunctions.h"
#include "linkedListIntr.h"

// macro definitions below

// container to store the bloom filters and the skiplists
typedef struct monitorContainter
{
    HT persons;
    HT bloom_filters;
    HT skip_lists_vaccinated;
    HT skip_lists_not_vaccinated;
    lList viruses;
    lList countries;
    int bloom_size;
} *mon_con;

// this is going to represent the vaccination status of a person for a certain virus
typedef struct personVaccination
{
    pInfo person;
    bool isVaccinated;
    char *date;
} *personVac;

// int functions
// compares the struct with the given id
int comparePersonVaccinationData(Item data, Item id);

// reads command line arguments if given
int argc_argv_manipulator_mon(int argc, char **argv, char **read_path, char **write_path);

// checks if the person info are the same with the given file input
int comparePersonInfo(Item person, char *firstName, char *lastName, char *country, unsigned short int age);

int updateDataStructs(mon_con *container, char **input_dirs, int amount_of_sub_dirs, char **read_files, int amount_of_files);

// void functions
void mon_general_initialization(void);
void monitor_initialization(char *write_p, char *read_p, int *write_fd, int *read_fd, struct pollfd *files_descs, mon_con *container, char ***input_dirs, char ***read_files, int *amount_of_sub_dirs, int *amount_of_files, int *buffer_size, int *blm_size);
void initContainer(mon_con *container, int bloom_size);
void initDataStructs(mon_con *container, FILE **stream, char *country);
void destroyPersonVaccination(Item discard);
void printPersonVaccinationData(Item data);
void initPollfdSturctMon(char *write_p, char *read_p, int *write_fd, int *read_fd, struct pollfd *files_descs);
void readBufferAndBloomSize(int *buffer_size, int *blm_size, struct pollfd *files_descs);
void readSubdirsAmount(int *amount_of_sub_dirs, int buffer_size, struct pollfd *files_descs);
void sendBlooms(mon_con *container, struct pollfd *files_descs, int buffer_size, int blm_size);
void destroyMonConData(mon_con discard);
void wait_for_stop(struct pollfd *fds, int buf_size);

// personVac functions below
personVac createPersonVaccinationData(pInfo person, bool isVaccinated, char *date);

// char functions below
char **readSubDirs(int amount_of_subdirs, int buffer_size, struct pollfd *files_descs);
char **openSubDirs(mon_con *container, char **input_dirs, int amount_of_subdirs, int *files_size);
