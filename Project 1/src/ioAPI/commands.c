// file: commands.c
// this file contains the implementation of the commands for mngstd
// libraries below
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// custom headers below
#include "hashTableIntr.h"
#include "commandsIntr.h"
#include "commonIntr.h"

// macro defitions
#define AGEGROUPS 4
#define TILL20 1
#define TILL40 2
#define TILL60 3
#define PLUS60 4

// called when the appropriate command is given
void vaccineStatusBloomSequence(vacData *container, char *citizenID, char *virusName)
{
    // get the associated bloom filter
    blm bloom = getEntry((*container)->bloom_filters, virusName);

    // check if the bloom filter exists and if not print error message and exit the function
    if (!bloom)
    {
        printf("ERROR: NON EXISTING VIRUS\n");
        return;
    }

    // check if the person exists in the apps database
    if(!doesKeyExists((*container)->persons, citizenID))
    {
        printf("ERROR: NON EXISTING CITIZEN\n");
        return;
    }

    // look at filter in order to see if the person is vaccinated or not
    if (blmLookup(bloom, citizenID)) printf("MAYBE\n");
    else printf("NOT VACCINATED\n");

    return;
}

void vaccineStatus(vacData *container, char *citizenID, char *virusName)
{
    sList skip_list;
    personVac tempData;
    lNode node;
    pInfo prsn = (pInfo)getEntry((*container)->persons, citizenID);

    // create temporary data to search in the skip list
    if (prsn) tempData = createPersonVaccinationData(prsn, true, null);
    else
    { 
        printf("ERROR: NON EXISTING CITIZEN\n");
        return;
    }

    if (virusName)
    {
        // find the correct skip list
        skip_list = (sList)getEntry((*container)->skip_list_vaccinated, virusName);

        // search the skip list for the given person
        if (skip_list) node = skipListSearch(skip_list, tempData);
        else node = null;
        
        // get the actual data if the person existed in the skip list
        // if not he's not vaccinated, print the appropriate message
        if(node) printf("VACCINATED ON %s\n", ((personVac)getDataNode(node))->date);
        else printf("NOT VACCINATED\n");
    }
    else
    {
        // get the amount of viruses in the app right now
        int listSize = getListSize((*container)->viruses);
        
        // iterate through the viruses list to check the specified person for all the viruses
        for(int i = 0; i < listSize; i++)
        {
            // extract the virus from the viruses list
            Item virus = getDataNode(getListNodeAt((*container)->viruses, i));

            // extract the appropriate skip list from the vaccinated skip list hash table
            skip_list = (sList)getEntry((*container)->skip_list_vaccinated, (char *)virus);

            // search for the person in the skip list
            if (skip_list) node = skipListSearch(skip_list, tempData);
            else node = null;

            // if the node is not null then the person is vaccinated
            // if the node is null the person is not vaccinated
            // print the appropriate message for each case
            if(node) printf("%s YES %s\n", (char *)virus, ((personVac)getDataNode(node))->date);
            else printf("%s NO\n", (char *)virus);
        }
    }

    // deallocate memory needed by temporary variable
    free(tempData);

    return;
}

void popStatusSequence(vacData *container, char *country, char *virusName, char *date1, char *date2)
{
    lNode node;
    sList skip_list;
    lList list;
    int lowerSize, upperSize;
    int vaccinatedPersonsSum;

    // check if there are two dates given
    if (!(date1 && date2))
    {
        printf("ERROR\n");
        return;
    }

    // 0 -> day, 1 -> month, 2 -> year
    char **lowerBound = stringParser(date1, &lowerSize, "-");
    char **upperBound = stringParser(date2, &upperSize, "-");
    skip_list = getEntry((*container)->skip_list_vaccinated, virusName);

    // check if skip list for the given virus exists
    if(!skip_list)
    {
        printf("ERROR: NON EXISTING VIRUS\n");
        return;
    }

    // if a country is given then check population status for the specific country
    if (country)
    {
        // double check if the given country exists in the apps data base or not
        node = findList((*container)->countries, country);
        if (!node)
        {
            printf("ERROR: NON EXISTING COUNTRY\n");
            return;
        }

        // initialize vaccinated persons counter
        vaccinatedPersonsSum = 0;

        // get the base layer of the skip list associated with the given virus
        // to search each person
        list = skip_list->layers[0];

        // get the list head
        lNode iterator = getListHead(list);

        // iterate through the list until we reach the end of the list
        while (iterator != null)
        {
            // check if the persons country is the one we're intrested in
            if (!strcmp(((personVac)getDataNode(iterator))->person->country, country))
            {
                // vairable to store the vaccination date size
                int vacDateSize = 0;

                // copying the contents of the date to avoid undefined behaviour of the app if a date is deleted
                char *tempString = malloc((strlen(((personVac)getDataNode(iterator))->date) + 1) * sizeof(char));
                strcpy(tempString, ((personVac)getDataNode(iterator))->date);

                // parsing the string got above to check individualy for the date range
                char **vaccinationDate = stringParser(tempString, &vacDateSize, "-");

                // check if the person is vaccinated between the given range
                // if the person is vaccinated between the given range update vaccinated person counter
                if ((atoi(vaccinationDate[2]) >= atoi(lowerBound[2]) || atoi(vaccinationDate[2]) <= atoi(upperBound[2])) && // check for year
                (atoi(vaccinationDate[1]) >= atoi(lowerBound[1]) || atoi(vaccinationDate[1]) <= atoi(upperBound[1])) && // check for month
                (atoi(vaccinationDate[0]) >= atoi(lowerBound[0]) || atoi(vaccinationDate[0]) <= atoi(upperBound[0]))) // check for date
                {
                    vaccinatedPersonsSum++;
                }

                // deallocated memory to avoid memory leaks
                deleteParsedString(vaccinationDate, vacDateSize);
                free(tempString);
            }

            // continue with the rest of the list
            iterator = getListNext(iterator);
        }
        
        // print the result
        printf("%s %d %.1f%%\n", country, vaccinatedPersonsSum, ((vaccinatedPersonsSum) / (((countryInfo)getDataNode(node))->population * 1.0)) * 100.0);
    }
    else
    {
        // if no country is given we do the same thing for each country
        lNode currNode;
        
        // get the list size of the countries list
        int listSize = getListSize((*container)->countries);
        
        for(int i = 0; i < listSize; i++)
        {
            // initialize the persons vaccinated counter
            vaccinatedPersonsSum = 0;

            // get the first i'th country in the list
            currNode = getListNodeAt((*container)->countries, i);

            // get the base layer of the skip list associated with the virus
            list = skip_list->layers[0];
            
            // get the head of the list
            lNode iterator = getListHead(list);

            // get the temporary country info node
            countryInfo cInfo = getDataNode(currNode);

            // iterate through the list until we reach the end of the list
            while (iterator != null)
            {
                // check if the persons country is the one we're intrested in
                if (!strcmp(((personVac)getDataNode(iterator))->person->country, cInfo->country))
                {
                    // vairable to store the vaccination date size
                    int vacDateSize = 0;

                    // copying the contents of the date to avoid undefined behaviour of the app if a date is deleted
                    char *tempString = malloc((strlen(((personVac)getDataNode(iterator))->date) + 1) * sizeof(char));
                    strcpy(tempString, ((personVac)getDataNode(iterator))->date);

                    // parsing the string got above to check individualy for the date range
                    char **vaccinationDate = stringParser(tempString, &vacDateSize, "-");

                    // check if the person is vaccinated between the given range
                    // if the person is vaccinated between the given range update vaccinated person counter
                    if ((atoi(vaccinationDate[2]) >= atoi(lowerBound[2]) || atoi(vaccinationDate[2]) <= atoi(upperBound[2])) && // check for year
                    (atoi(vaccinationDate[1]) >= atoi(lowerBound[1]) || atoi(vaccinationDate[1]) <= atoi(upperBound[1])) && // check for month
                    (atoi(vaccinationDate[0]) >= atoi(lowerBound[0]) || atoi(vaccinationDate[0]) <= atoi(upperBound[0]))) // check for date
                    {
                        vaccinatedPersonsSum++;
                    }

                    // deallocated memory to avoid memory leaks
                    deleteParsedString(vaccinationDate, vacDateSize);
                    free(tempString);
                }

                // continue with the rest of the list
                iterator = getListNext(iterator);
            }
            
            // print the result
            printf("%s %d %.1f%%\n", cInfo->country, vaccinatedPersonsSum, ((vaccinatedPersonsSum) / (cInfo->population * 1.0)) * 100.0);
        }
    }

    // delete allocated memoery for the given dates
    deleteParsedString(lowerBound, lowerSize);
    deleteParsedString(upperBound, upperSize);

    return;
}

void popStatusByAgeSequence(vacData *container, char *country, char *virusName, char *date1, char *date2)
{
    lNode node;
    sList skip_list;
    lList list;
    int lowerSize, upperSize;
    int ageGroups[AGEGROUPS] = {0};
    float percentage;


    // check if there are two dates given
    if (!(date1 && date2))
    {
        printf("ERROR\n");
        return;
    }

    // 0 -> day, 1 -> month, 2 -> year
    char **lowerBound = stringParser(date1, &lowerSize, "-");
    char **upperBound = stringParser(date2, &upperSize, "-");
    skip_list = getEntry((*container)->skip_list_vaccinated, virusName);

    // check if skip list for the given virus exists
    if(!skip_list)
    {
        printf("ERROR: NON EXISTING VIRUS\n");
        return;
    }

    if (country)
    {
        // get the country from the list and check if it exists in the apps database
        node = findList((*container)->countries, country);
        if (!node)
        {
            printf("ERROR: NON EXISTING COUNTRY\n");
            return;
        }

        // get the base layer of the skip list
        list = skip_list->layers[0];
        
        // get the head of the list
        lNode iterator = getListHead(list);

        // get the temporary country info node
        countryInfo cInfo = getDataNode(node);

        // iterate through the list until the end of it
        while (iterator != null)
        {
            if (!strcmp(((personVac)getDataNode(iterator))->person->country, country))
            {
                // vairable to store the vaccination date size
                int vacDateSize = 0;

                // copying the contents of the date to avoid undefined behaviour of the app if a date is deleted
                char *tempString = malloc((strlen(((personVac)getDataNode(iterator))->date) + 1) * sizeof(char));
                strcpy(tempString, ((personVac)getDataNode(iterator))->date);

                // parsing the string got above to check individualy for the date range
                char **vaccinationDate = stringParser(tempString, &vacDateSize, "-");

                // check if the person is vaccinated between the given range
                // if the person is vaccinated between the given range update vaccinated person counter for the right age group
                if ((atoi(vaccinationDate[2]) >= atoi(lowerBound[2]) || atoi(vaccinationDate[2]) <= atoi(upperBound[2])) && // check for year
                (atoi(vaccinationDate[1]) >= atoi(lowerBound[1]) || atoi(vaccinationDate[1]) <= atoi(upperBound[1])) && // check for month
                (atoi(vaccinationDate[0]) >= atoi(lowerBound[0]) || atoi(vaccinationDate[0]) <= atoi(upperBound[0]))) // check for date
                {
                    // get the age of the person
                    int age = getAge(((personVac)getDataNode(iterator))->person);

                    // search for the right age group
                    if (age <= 20) ageGroups[TILL20 - 1]++;
                    else if (age <= 40) ageGroups[TILL40 - 1]++;
                    else if (age <= 60) ageGroups[TILL60 - 1]++;
                    else ageGroups[PLUS60 - 1]++;
                }

                // delete the allocated memory to avoid leaks
                deleteParsedString(vaccinationDate, vacDateSize);
                free(tempString);
            }
            
            // continue with the rest of the list
            iterator = getListNext(iterator);
        }

        // print the result
        printf("%s\n", country);
        for(int i = 0; i < AGEGROUPS; i++)
        {
            switch (i)
            {
                case TILL20 - 1:
                    if(!cInfo->till20) percentage = 0.0f;
                    else percentage = ((ageGroups[TILL20 - 1]) / (cInfo->till20 * 1.0)) * 100.0;
                    printf("0-20 %d %.1f%%\n", ageGroups[TILL20 - 1], percentage);
                    break;
                case TILL40 - 1:
                    if(!cInfo->till40) percentage = 0.0f;
                    else percentage = ((ageGroups[TILL40 - 1]) / (cInfo->till40 * 1.0)) * 100.0;
                    printf("20-40 %d %.1f%%\n", ageGroups[TILL40 - 1], percentage);
                    break;
                case TILL60 - 1:
                    if(!cInfo->till60) percentage = 0.0f;
                    else percentage = ((ageGroups[TILL60 - 1]) / (cInfo->till60 * 1.0)) * 100.0;
                    printf("40-60 %d %.1f%%\n", ageGroups[TILL60 - 1], percentage);
                    break;
                case PLUS60 - 1:
                    if(!cInfo->olderThan60) percentage = 0.0f;
                    else percentage = ((ageGroups[PLUS60 - 1]) / (cInfo->olderThan60 * 1.0)) * 100.0;
                    printf("60+ %d %.1f%%\n", ageGroups[PLUS60 - 1], percentage);
                    break;
            }
        }
    }
    else
    {
        // if no country is given do the same thing for each country
        lNode currNode;
        
        // get the countries list size
        int listSize = getListSize((*container)->countries);
        
        // repeat for each country
        for(int i = 0; i < listSize; i++)
        {
            // we have to initialize every age group counter every time we do this operation
            // for each countries
            for(int j = 0; j < AGEGROUPS; j++) ageGroups[j] = 0;

            // get the i'th country
            currNode = getListNodeAt((*container)->countries, i);

            // get the base layer of the skip list associated with the virus
            list = skip_list->layers[0];

            // get the head of the list
            lNode iterator = getListHead(list);

            // get the temporary country info node
            countryInfo cInfo = getDataNode(currNode);

            // iterate through the list until the end of it
            while (iterator != null)
            {
                if (!strcmp(((personVac)getDataNode(iterator))->person->country, cInfo->country))
                {
                    // vairable to store the vaccination date size
                    int vacDateSize = 0;

                    // copying the contents of the date to avoid undefined behaviour of the app if a date is deleted
                    char *tempString = malloc((strlen(((personVac)getDataNode(iterator))->date) + 1) * sizeof(char));
                    strcpy(tempString, ((personVac)getDataNode(iterator))->date);

                    // parsing the string got above to check individualy for the date range
                    char **vaccinationDate = stringParser(tempString, &vacDateSize, "-");

                    // check if the person is vaccinated between the given range
                    // if the person is vaccinated between the given range update vaccinated person counter for the right age group
                    if ((atoi(vaccinationDate[2]) >= atoi(lowerBound[2]) || atoi(vaccinationDate[2]) <= atoi(upperBound[2])) && // check for year
                    (atoi(vaccinationDate[1]) >= atoi(lowerBound[1]) || atoi(vaccinationDate[1]) <= atoi(upperBound[1])) && // check for month
                    (atoi(vaccinationDate[0]) >= atoi(lowerBound[0]) || atoi(vaccinationDate[0]) <= atoi(upperBound[0]))) // check for date
                    {
                        // get the age of the person
                        int age = getAge(((personVac)getDataNode(iterator))->person);

                        // search for the right age group
                        if (age <= 20) ageGroups[TILL20 - 1]++;
                        else if (age <= 40) ageGroups[TILL40 - 1]++;
                        else if (age <= 60) ageGroups[TILL60 - 1]++;
                        else ageGroups[PLUS60 - 1]++;
                    }

                    // delete the allocated memory to avoid leaks
                    deleteParsedString(vaccinationDate, vacDateSize);
                    free(tempString);
                }

                // continue with the rest of the list
                iterator = getListNext(iterator);
            }
            
            // print the result
            printf("%s\n", cInfo->country);
            for(int i = 0; i < AGEGROUPS; i++)
            {
                switch (i)
                {
                    case TILL20 - 1:
                        if(!cInfo->till20) percentage = 0.0f;
                        else percentage = ((ageGroups[TILL20 - 1]) / (cInfo->till20 * 1.0)) * 100.0;
                        printf("0-20 %d %.1f%%\n", ageGroups[TILL20 - 1], percentage);
                        break;
                    case TILL40 - 1:
                        if(!cInfo->till40) percentage = 0.0f;
                        else percentage = ((ageGroups[TILL40 - 1]) / (cInfo->till40 * 1.0)) * 100.0;
                        printf("20-40 %d %.1f%%\n", ageGroups[TILL40 - 1], percentage);
                        break;
                    case TILL60 - 1:
                        if(!cInfo->till60) percentage = 0.0f;
                        else percentage = ((ageGroups[TILL60 - 1]) / (cInfo->till60 * 1.0)) * 100.0;
                        printf("40-60 %d %.1f%%\n", ageGroups[TILL60 - 1], percentage);
                        break;
                    case PLUS60 - 1:
                        if(!cInfo->olderThan60) percentage = 0.0f;
                        else percentage = ((ageGroups[PLUS60 - 1]) / (cInfo->olderThan60 * 1.0)) * 100.0;
                        printf("60+ %d %.1f%%\n", ageGroups[PLUS60 - 1], percentage);
                        break;
                }
            }
            printf("\n");
        }
    }

    // deallocate memory to avoid memory leaks
    deleteParsedString(lowerBound, lowerSize);
    deleteParsedString(upperBound, upperSize);

    return;
}

void insertCitizenSequence(vacData *container, char *citizenID, char *firstName, char *lastName, char *country, int age, char *virusName, char *isVaccinated, char *date)
{
    sList skip_list, skip_list_not_vac;
    personVac tempData;
    lNode node;
    blm bloom;
    countryInfo cInfo;
    char *virus;

    // check if the country exists in the apps database
    node = findList((*container)->countries, country);
    if (!node)
    {
        // create country info and initialize it's values
        cInfo = malloc(sizeof(*cInfo));
        cInfo->country = createString(country);
        cInfo->population = 0;
        cInfo->till20 = 0;
        cInfo->till40 = 0;
        cInfo->till60 = 0;
        cInfo->olderThan60 = 0;

        // insert it in the list
        insertList((*container)->countries, cInfo);
    }
    else
    {
        cInfo = getDataNode(node);
    }

    // check if pesron information are the same
    pInfo person = (pInfo)getEntry((*container)->persons, citizenID);
    if (!person)
    {
        // insert the person in the person's hash, ie the apps database
        person = createPerson(citizenID, firstName, lastName, cInfo->country, age);
        insertHash((*container)->persons, person, (*container)->persons->hashFunc(getCitizenId(person)));

        // update population counter for the country given
        cInfo->population++;
    }
    else if (strcmp(person->firstName, firstName) || strcmp(person->lastName, lastName) || strcmp(person->country, country) || person->age != age)
    {
        // if the information is different print the appropriate message to the user
        printf("ERROR: PERSON INFROMATION DIFFER\n");
        return;
    }

    // check if the person is vaccinated but no date is given
    if (!strcmp(isVaccinated, "YES") && !date)
    {
        printf("ERROR: NO DATE GIVEN\n");
        return;
    }

    // check if the person not vaccinated but a date is given
    if (!strcmp(isVaccinated, "NO") && date)
    {
        printf("ERROR: DATE GIVEN\n");
        return;
    }

    // check if the virus exists in the apps database
    node = findList((*container)->viruses, virusName);
    if (!node)
    {
        virus = createString(virusName);
        insertList((*container)->viruses, virus);
    }
    else
    {
        virus = (char *)getDataNode(node);
    }

    // create temporary data to search in the skip list
    tempData = createPersonVaccinationData(person, true, null);

    // find the correct skip list
    skip_list = (sList)getEntry((*container)->skip_list_vaccinated, virus);

    // search the skip list for the given person
    if (skip_list) node = skipListSearch(skip_list, tempData);
    else node = null;

    // if the node is not null then the person is aalready vaccinated to the virus
    if(node)
    {
        printf("ERROR: CITIZEN %s ALREADY VACCINATED ON %s\n", getCitizenId(((personVac)getDataNode(node))->person), ((personVac)getDataNode(node))->date);
    }
    else
    {
        // if the isVaccinated string is yes then make the correct insertion in the skips and blooms
        if (!strcmp(isVaccinated, "YES"))
        {
            // if the citizen id is new create a new skip list and hash entry
            if (!doesKeyExists((*container)->skip_list_vaccinated, virus))
            {
                // create a new skip list and bloom filter, the given virus did not exist in the app until now
                skip_list = createSkipList(virus, destroyPersonVaccination, printPersonVaccinationData, comparePersonVaccinationData);
                bloom = createFilter(virus, (*container)->bloom_size, hash_i);

                // insert the data in the two data structs
                insertSkipList(skip_list, createPersonVaccinationData(person, true, date));
                blmInsert(bloom, getCitizenId(person));

                // insert the two data structs (pointers) in the proper hash tables
                insertHash((*container)->skip_list_vaccinated, skip_list, (*container)->skip_list_vaccinated->hashFunc(virus));
                insertHash((*container)->bloom_filters, bloom, (*container)->bloom_filters->hashFunc(virus));
            }
            else
            {
                // if not just insert the person in the correct hash entry of the virus
                insertSkipList(getEntry((*container)->skip_list_vaccinated, virus), createPersonVaccinationData(person, true, date));
                blmInsert(getEntry((*container)->bloom_filters, virus), getCitizenId(person));
            }
        }
        else
        {
            // if the citizen id is new create a new skip list and hash entry
            if (!doesKeyExists((*container)->skip_list_not_vaccinated, virus))
            {
                // create only a new skip list for the not vaccinated person and repeat the same steps as above
                skip_list_not_vac = createSkipList(virus, destroyPersonVaccination, printPersonVaccinationData, comparePersonVaccinationData);
                insertSkipList(skip_list_not_vac, createPersonVaccinationData(person, false, null));
                insertHash((*container)->skip_list_not_vaccinated, skip_list_not_vac, (*container)->skip_list_not_vaccinated->hashFunc(virus));
            }
            else
            {
                // if not just insert the person in the correct hash entry of the virus
                insertSkipList(getEntry((*container)->skip_list_not_vaccinated, virus), createPersonVaccinationData(person, false, null));
            }
        }
    }

    free(tempData);
    return;
}

void vaccinateNowSequence(vacData *container, char *citizenID, char *firstName, char *lastName, char *country, int age, char *virusName)
{
    time_t today;
    sList skip_list, skip_list_not_vac;
    personVac tempData, toInsert;
    lNode node;
    blm bloom;
    char *virus;

    // check if pesron information are the same
    // or if person exists
    pInfo person = (pInfo)getEntry((*container)->persons, citizenID);
    if (!person)
    {
        printf("ERROR: PERSON DOES NOT EXIST IN DATABASE\n");
        return;
    }

    if (strcmp(person->firstName, firstName) || strcmp(person->lastName, lastName) || strcmp(person->country, country) || person->age != age)
    {
        printf("ERROR: PERSON INFROMATION DIFFER\n");
        return;
    }

    // check if the virus exists in the apps database
    node = findList((*container)->viruses, virusName);
    if (!node)
    {
        virus = createString(virusName);
        insertList((*container)->viruses, virus);
    }
    else
    {
        virus = (char *)getDataNode(node);
    }
    
    // get the local date
    // get current time
    time(&today);

    // get current local time
    struct tm *day = localtime(&today);

    // get the full length needed by the date format
    int len = snprintf( NULL, 0, "%d-%d-%d", day->tm_mday, day->tm_mon + 1, day->tm_year + 1900);

    // create a string to store it
    char fulldate[len + 1];

    // convert above ints to string
    // int to string conversion
    // due to project simplicity a month has 30 days, if day->tm_mday is 31 subtract one from it
    if (day->tm_mday == 31) day->tm_mday--;
    snprintf(fulldate, len + 1, "%d-%d-%d", day->tm_mday, day->tm_mon + 1, day->tm_year + 1900);

    // create temporary data to search in the skip list
    tempData = createPersonVaccinationData(person, true, null);

    // find the correct skip list
    skip_list = (sList)getEntry((*container)->skip_list_vaccinated, virus);

    // search the skip list for the given person
    if (skip_list) node = skipListSearch(skip_list, tempData);
    else node = null;

    if(node)
    {
        printf("ERROR: CITIZEN %s ALREADY VACCINATED ON %s\n", getCitizenId(((personVac)getDataNode(node))->person), ((personVac)getDataNode(node))->date);
    }
    else
    {
        // insert the vaccination on the bloom filter
        bloom = (blm)getEntry((*container)->bloom_filters, virus);

        if (!bloom)
        {
            bloom = createFilter(virus, (*container)->bloom_size, hash_i);
            blmInsert(bloom, citizenID);
            insertHash((*container)->bloom_filters, bloom, (*container)->bloom_filters->hashFunc(virus));
        }
        else blmInsert(bloom, citizenID);

        // get the not vaccinated skip list
        skip_list_not_vac = (sList)getEntry((*container)->skip_list_not_vaccinated, virus);

        // searching for the virus
        if(skip_list_not_vac)
        {
            if ((toInsert = (personVac)getDataNode(skipListSearch(skip_list_not_vac, tempData))))
            {
                deleteFromSkipList(skip_list_not_vac, toInsert);
            }
        }
        if(!getEntry((*container)->skip_list_vaccinated, virus))
        {
            skip_list = createSkipList(virus, destroyPersonVaccination, printPersonVaccinationData, comparePersonVaccinationData);
            insertSkipList(skip_list, createPersonVaccinationData(person, true, fulldate));
            insertHash((*container)->skip_list_vaccinated, skip_list, (*container)->skip_list_vaccinated->hashFunc(virus));
        }
        else insertSkipList(getEntry((*container)->skip_list_vaccinated, virus), createPersonVaccinationData(person, true, fulldate));
    }

    free(tempData);
    return;
}

void listNonVacPersonsSequence(vacData *container, char *virusName)
{
    // extract the correct skip list from the not vaccinated skip list hash table
    sList skip_list = (sList)getEntry((*container)->skip_list_not_vaccinated, virusName);

    // check if the skip list exists
    if (!skip_list)
    {
        printf("ERROR: NO ENTRIES\n");
        return;
    }

    // get the list size
    int listSize = getListSize(skip_list->layers[0]);

    for(int i = 0; i < listSize; i++)
    {
        // get the i'th person from the base layer of the skip list and print the information
        pInfo person = ((personVac)getDataNode(getListNodeAt(skip_list->layers[0], i)))->person;
        printPersonInformation(person);
    }

    return;
}

void exitSequence(vacData *container, FILE **inputStream)
{
    // delete allocated memory of the container
    destroyVacData(*container);

    // close file
    if (*inputStream) fclose(*inputStream);
    return;
}