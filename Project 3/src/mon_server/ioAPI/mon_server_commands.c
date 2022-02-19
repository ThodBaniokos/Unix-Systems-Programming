// file: mon_commands.c
// implementation of the monitor process side of the queries
// libs below
#include <stdio.h>
#include <string.h>

// custom headers below
#include "utilsIntr.h"
#include "answerIntr.h"
#include "generalIntr.h"
#include "convertersIntr.h"
#include "mon_server_commandsIntr.h"
#include "monitorServerHelpersIntr.h"

// globals used at travel request as answers
char *accepted = "REQUEST ACCEPTED - HAPPY TRAVELS";
char *rejected = "REQUEST REJECTED - YOU ARE NOT VACCINATED";
char *rej_vaccinated = "REQUEST REJECTED - YOU WILL NEED ANOTHER VACCINATION BEFORE TRAVEL DATE";

// travel request query
char *travel_req(mon_con *container, char *query, req_stats *stats)
{
    // travel request query format
    // /travelRequest citizenID date countryFrom countryTo virusName
    //       0            1       2       3          4         5
    // parse the given query
    int size;
    char **input = stringParser(query, &size, " ");

    // get the person indicated by the citizenID
    pInfo person = (pInfo)getEntry((*container)->persons, input[1]);

    // auxiliary variables
    personVac tempData, actualData;
    sList skip_list;

    // if the person exists create a temporary person vaccination struct variable to search in the skip list
    // if not create an answer indicating that a person with the given citizenID does not exist
    if (person) tempData = createPersonVaccinationData(person, true, null);
    else
    {
        deleteParsedString(input, size);
        return build_ans("ERROR: NON EXISTING CITIZEN IN DATABASE", (int)strlen("ERROR: NON EXISTING CITIZEN IN DATABASE"));
    }

    // search in the vaccinated persons skip list to find the skip list of the given virus
    skip_list = (sList)getEntry((*container)->skip_lists_vaccinated, input[5]);

    // if the skip list exists try to find the data stored for the person
    // if not the person is not vaccinated to the given virus, build the appropriate answer
    if(skip_list) actualData = (personVac)getDataNode(skipListSearch(skip_list, tempData));
    else
    {
        if (tempData) free(tempData);
        deleteParsedString(input, size);

        // update the query statistics
        (*stats).rejected++;
        (*stats).total++;

        return build_ans(rejected, (int)strlen(rejected));
    }

    // check if the vaccination date is less than 6 months appart of the travel date
    // if not then build the appropriate answer
    if (valid_vac(actualData->date, input[2]) == false)
    {
        if (tempData) free(tempData);
        deleteParsedString(input, size);

        // update the query statistics
        (*stats).rejected++;
        (*stats).total++;

        return build_ans(rej_vaccinated, (int)strlen(rej_vaccinated));
    }

    // if we've reached this point in the function then the person can travel
    // build the appropriate message
    // update the query statistics
    (*stats).accepted++;
    (*stats).total++;

    if (tempData) free(tempData);
    deleteParsedString(input, size);
    return build_ans(accepted, (int)strlen(accepted));

}

// search vaccination status query
char *search_vac_status(mon_con *container, char *query)
{
    // search vaccination status query format
    // /searchVaccinationStatus citizenID
    //             0                1
    // parse the given query
    int size;
    char **input = stringParser(query, &size, " ");

    // get the person indicated by citizenID
    pInfo person = (pInfo)getEntry((*container)->persons, input[1]);

    // auxiliary functions
    personVac tempData, actualData;
    sList skip_list;

    // answer variable
    char *ans = null;

    // check if person exists
    if (person)
    {
        // if the person exists create temporary information and build the first part of the answer
        tempData = createPersonVaccinationData(person, true, null);

        // copy the info of the person in the answer
        ans = copyPersonInformation(person);

    }
    else
    {
        deleteParsedString(input, size);
        return null;
    }

    // start iterating through every virus in the vaccinated skip list to see if the person is vaccinated or not
    lNode iterator = getListHead((*container)->viruses);

    // iterate till the end of the list
    while (iterator != null)
    {
        // get the virus stored in the list
        char *virus = (char *)getDataNode(iterator);

        // error checking
        if (!virus) break;

        // get the skip list related to the virus
        skip_list = (sList)getEntry((*container)->skip_lists_vaccinated, virus);

        // if the skip list exists get the node related to the person
        // if not continue to the next virus and check
        if(skip_list) actualData = (personVac)getDataNode(skipListSearch(skip_list, tempData));
        else
        {
            iterator = getListNext(iterator);
            continue;
        }

        // if there's no record of the person for this virus we're not reporting anything
        // since the application does not know anything about the person and the virus
        if (!actualData)
        {
            iterator = getListNext(iterator);
            continue;
        }

        // if the actual data exeists build the answer
        ans = setVaccinationStatus(ans, virus, actualData->date);

        iterator = getListNext(iterator);
    }

    iterator = getListHead((*container)->viruses);

    while (iterator != null)
    {
        // get the virus stored in the list
        char *virus = (char *)getDataNode(iterator);

        // error checking
        if (!virus) break;

        // get the skip list related to the virus
        skip_list = (sList)getEntry((*container)->skip_lists_not_vaccinated, virus);

        // if the skip list exists get the node related to the person
        // if not continue to the next virus and check
        if(skip_list) actualData = (personVac)getDataNode(skipListSearch(skip_list, tempData));
        else
        {
            iterator = getListNext(iterator);
            continue;
        }

        // if there's no record of the person for this virus we're not reporting anything
        // since the application does not know anything about the person and the virus
        if (!actualData)
        {
            iterator = getListNext(iterator);
            continue;
        }

        // if the actual data exeists build the answer
        ans = setNonVaccinationStatus(ans, virus);

        iterator = getListNext(iterator);
    }

    if (tempData) destroyPersonVaccination(tempData);
    deleteParsedString(input, size);

    // return the answer
    return ans;

}
