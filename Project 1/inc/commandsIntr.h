#pragma once

#include "Types.h"
#include "vaccineMonitorHelpersIntr.h"

// commands of the vaccineMonitor app
void vaccineStatusBloomSequence(vacData *container, char *citizenID, char *virusName);
void vaccineStatus(vacData *container, char *citizenID, char *virusName);
void popStatusSequence(vacData *container, char *country, char *virusName, char *date1, char *date2);
void popStatusByAgeSequence(vacData *container, char *country, char *virusName, char *date1, char *date2);
void insertCitizenSequence(vacData *container, char *citizenID, char *firstName, char *lastName, char *country, int age, char *virusName, char *isVaccinated, char *date);
void vaccinateNowSequence(vacData *container, char *citizenID, char *firstName, char *lastName, char *country, int age, char *virusName);
void listNonVacPersonsSequence(vacData *container, char *virusName);
void exitSequence(vacData *container, FILE **inputStream);