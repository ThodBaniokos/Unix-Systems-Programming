// file: generalIntr.h
// interface of general tools used by monitor processes
// include guard
#pragma once

// custom headers below
#include "Types.h"
#include "personIntr.h"

// bool functions below
bool valid_vac(char *k_vac_date, char *travel_date);
bool isFilePresent(char **already_read_files, int size, char *to_read);

// char functions
char *copyPersonInformation(pInfo person);
char *setVaccinationStatus(char *soFarAns, char *virus, char *date);
char *setNonVaccinationStatus(char *soFarAns, char *virus);