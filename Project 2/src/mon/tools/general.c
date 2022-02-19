// file: general.c
// implementation of the general tools used by monitor processes
// libs below
#include <stdio.h>
#include <string.h>

// custom headers below
#include "utilsIntr.h"
#include "generalIntr.h"
#include "convertersIntr.h"

// bool functions
bool valid_vac(char *k_vac_date, char *travel_date)
{
    // copy the vaccination date to avoid possible altreations in data
    char *vac_date = malloc((strlen(k_vac_date) + 1) + sizeof(char));

    // check for correct memory allocation
    assert(vac_date != null);

    // copy the string
    strcpy(vac_date, k_vac_date);

    // day month year
    //  0    1    2
    // parse both dates given
    int parsed_vac_date_size;
    char **parsed_vac_date = stringParser(vac_date, &parsed_vac_date_size, "-");

    int parsed_travel_date_size;
    char **parsed_travel_date = stringParser(travel_date, &parsed_travel_date_size, "-");

    // convert the dates to ints
    int vac_day = atoi(parsed_vac_date[0]), vac_month = atoi(parsed_vac_date[1]), vac_year = atoi(parsed_vac_date[2]);
    int travel_day = atoi(parsed_travel_date[0]), travel_month = atoi(parsed_travel_date[1]), travel_year = atoi(parsed_travel_date[2]);

    int diff_year;
    int diff_month;
    int diff_day;

    // if the vaccination day is greater than the travel day, borrow a month, due to project simplicity a month has 30 days
    // subtract one month from travel month to get correct results
    if (travel_day < vac_day)
    {
        travel_day += 30;
        travel_month--;
    }

    // do the same as above but for the months and subtract from the year if needed
    if (travel_month < vac_month)
    {
        travel_month += 12;
        travel_year--;
    }

    // calculate difference
    diff_day = travel_day - vac_day;
    diff_month = travel_month - vac_month;
    diff_year = travel_year - vac_year;

    // delete the parsed strings
    deleteParsedString(parsed_travel_date, parsed_travel_date_size);
    deleteParsedString(parsed_vac_date, parsed_vac_date_size);

    // check for the 6 month range
    if (diff_year > 0) return false;
    if (diff_month > 6) return false;
    if (diff_day > 30 && diff_month == 5) return false;

    // if none of the above is true return true
    return true;
}

bool isFilePresent(char **already_read_files, int size, char *to_read)
{
    // check if the files read are present
    for(int i = 0; i < size; i++)
    {
        // if the file is present return true
        if (!strcmp(already_read_files[i], to_read)) return true;
    }

    // if the file is not present return false
    return false;
}

// char functions
char *copyPersonInformation(pInfo person)
{
    // get age to string
    char *age = int_to_string(getAge(person));

    // allocate enough memory to store all the information to string
    char *personInfo = calloc((strlen(getCitizenId(person)) + 1
    + strlen(getFirstName(person)) + 1
    + strlen(getLastName(person)) + 1
    + strlen(getCountry(person)) + 1
    + strlen("AGE") + 1
    + strlen(age) + 1), sizeof(char));

    // check for correct memory allocation
    assert(personInfo != null);

    // copy each field of the person to the string representing the information
    strcpy(personInfo, getCitizenId(person));
    strcat(personInfo, " ");
    strcat(personInfo, getFirstName(person));
    strcat(personInfo, " ");
    strcat(personInfo, getLastName(person));
    strcat(personInfo, " ");
    strcat(personInfo, getCountry(person));
    strcat(personInfo, "\n");
    strcat(personInfo, "AGE");
    strcat(personInfo, " ");
    strcat(personInfo, age);

    return personInfo;
}

char *setVaccinationStatus(char *soFarAns, char *virus, char *date)
{
    // allocate enough memory for the new ans
    char *temp = calloc((strlen(soFarAns) + 1
    + strlen(virus) + 1
    + strlen("VACCINATED ON ") + strlen(date) + 2), sizeof(char));

    // check for correct memory allocation
    assert(temp != null);

    // copy new information
    strcpy(temp, soFarAns);
    free(soFarAns);
    strcat(temp, "\n");
    strcat(temp, virus);
    strcat(temp, " ");
    strcat(temp, "VACCINATED ON ");
    strcat(temp, date);

    return temp;
}

char *setNonVaccinationStatus(char *soFarAns, char *virus)
{
    // allocate enough memory for new answer
    char *temp = calloc((strlen(soFarAns) + 1
    + strlen(virus) + 1
    + strlen("NOT YET VACCINATED") + 1), sizeof(char));

    // check for correct memory allocation
    assert(temp != null);

    // copy new information
    strcpy(temp, soFarAns);
    free(soFarAns);
    strcat(temp, "\n");
    strcat(temp, virus);
    strcat(temp, " ");
    strcat(temp, "NOT YET VACCINATED");

    return temp;
}