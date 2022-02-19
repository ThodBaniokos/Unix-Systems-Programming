// file: vaccinationMonitorHelpers.c
// implementation of the function helping the correct excecution of the app
// c libs
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

// custom libs
#include "vaccineMonitorHelpersIntr.h"

// implementations below
// personVac related functinos
// create an instance of this struct
personVac createPersonVaccinationData(pInfo person, bool isVaccinated, char *date)
{
    // allocate memory for the struct
    personVac data = malloc(sizeof(*data));

    // error handler
    assert(data != null);

    // copy the person given, to avoid data duplication because the person already exists in our app
    if (person) data->person = person;
    else data->person = null;

    // set if the person is vaccinated or not
    data->isVaccinated = isVaccinated;

    // date is given only if the above is true but we're checking for both to avoid errors
    if (date && data->isVaccinated) { data->date = malloc((strlen(date) + 1) * sizeof(char)); strcpy(data->date, date); }
    else data->date = null;
    
    return data;
}

// comapres the person vaccination data struct with the given id
// the identifier of the struct is the person citizen id
int comparePersonVaccinationData(Item data, Item id)
{
    int num1 = atoi(((personVac)data)->person->citizenID);
    int num2 = atoi(((personVac)id)->person->citizenID);
    return num1 - num2;
}

int compareCountry(Item info, Item country)
{
    return strcmp(((countryInfo)info)->country, country);
}

// prints the person vaccination data information
void printPersonVaccinationData(Item data)
{
    printf("%s : ", ((personVac)data)->person->citizenID);
    if (((personVac)data)->isVaccinated) printf("VACCINATED ON %s\n", ((personVac)data)->date);
    else printf("NOT VACCINATED\n");
}

// deletes allocated memory of the person vaccination data
void destroyPersonVaccination(Item discard)
{
    // if there is a date given delete it
    if (((personVac)discard)->date) free(((personVac)discard)->date);

    // delete the pointer
    free(discard);

    return;
}

// deletes allocated memory of the country info data
void destroyCountryInfo(countryInfo discard)
{
    // delete the allocated memory for the string representing the country
    if(discard->country) free(discard->country);

    // delete the pointer memory
    free(discard);


    return;
}

// command line input related
// reads command line arguments if given
int argc_argv_manipulator(int argc, char **argv, FILE **inputStream, int *bloom_size)
{
    // check if the arguments given are more than the programs name
    if(argc == 5)
    {
        // start reading from 1 because the first argv argument is the program name
        char *arg;
        bool correctArgs = false;
        for(int i = 1; i < argc; i++)
        {
            // getting the flag
            arg = argv[i];
            
            // checking the flags for input and config files
            // if there are these flags open the files given by the paths
            if(!strcmp(arg, "-c"))
            {
                correctArgs = true;
                if (access(argv[i + 1], F_OK) != 0) return ERROR;
                openIStream(inputStream, argv[i + 1]);
            }
            else if(!strcmp(arg, "-b"))
            {
                correctArgs = true;
                *bloom_size = atoi(argv[i + 1]);
            }
        }
        if (!correctArgs) return ERROR;
    }
    else return ERROR;

    // files opened succesfully or there are no arguments in command line
    return SUCCESS;
}

// compares the person info with the given one
int comparePersonInfo(Item person, char *firstName, char *lastName, char *country, unsigned short int age)
{
    // id is the same because of the hash table insertion
    return (!strcmp(((pInfo)person)->firstName, firstName) // first name must be the same
        && !strcmp(((pInfo)person)->lastName, lastName)  // last name must be the same
        && !strcmp(((pInfo)person)->country, country) // country must be the same
        && ((pInfo)person)->age == age);    // age must be the same
}

// opens input file stream
void openIStream(FILE **stream, char *path)
{
    // opens stream
    *stream = fopen(path, "r");

    // correct stream open check
    assert(*stream != null);

    return;
}

// container of the main function related functions
// deletes allocated memory of the container needed for the app
void destroyVacData(vacData dicsard)
{
    // delete the hash table of the persons
    destroyHash(dicsard->persons);
    
    // delete the hash table used for the bloom filters
    destroyHash(dicsard->bloom_filters);

    // delete the hash tables used for the skip lists
    destroyHash(dicsard->skip_list_vaccinated);
    destroyHash(dicsard->skip_list_not_vaccinated);

    // delete the lists
    destroyList(dicsard->countries);
    destroyList(dicsard->viruses);

    // delete the pointer itself
    free(dicsard);

    return;
}

// data structs related functions
// initializes the data structures needed for the app
void initDataStructs(vacData *container, FILE **stream)
{
    // input format: citizenID, firstName, lastName, country, age, virus, vaccinated status, date (if vaccinated status is YES)
    int size = 0, errors = 0;
    char *str = null;
    pInfo person;
    countryInfo cInfo, toUpdate;
    personVac tempData = createPersonVaccinationData(null, true, null);

    // allocate memory for the structs we're going to use in this assigment
    // hash table used to store all the persons
    (*container)->persons = createHash(hashFunction, printPersonInformation, deletePerson, compareCitizenId);

    // hash table used to store all the bloom filters
    (*container)->bloom_filters = createHash(hashFunction, null, blmDestroy, compareBloomKey);

    // hash table used to store the skip list of the vaccinated persons
    (*container)->skip_list_vaccinated = createHash(hashFunction, printSkipList, destroySkipList, compareSkipListKey);

    // hash table used to store the skip list of the not vaccinated persons       
    (*container)->skip_list_not_vaccinated = createHash(hashFunction, printSkipList, destroySkipList, compareSkipListKey);

    // list where the countries are going to be stored
    (*container)->countries = createList(null, compareCountry, printString, destroyCountryInfo);

    // list where the viruses are going to be stored
    (*container)->viruses = createList(null, compareStrings, printString, deletePointer);

    // repeat until the input stream is null, i.e. we've reached EOF (end of file)
    while (*stream != null)
    {
        // read the line and convert it to a string
        str = makeString(stream);

        // check if we've reached the end of the file
        if (!strlen(str)) break;

        // parse string with and seperate each word
        char **input = stringParser(str, &size, " ");
      
        // check if the given country is already in the right list
        // if not create it
        // if yes get it from the list
        if (!duplicateList((*container)->countries, input[3]))
        {
            // create country info and initialize it's values
            cInfo = malloc(sizeof(*cInfo));
            cInfo->country = createString(input[3]);
            cInfo->population = 0;
            cInfo->till20 = 0;
            cInfo->till40 = 0;
            cInfo->till60 = 0;
            cInfo->olderThan60 = 0;

            // insert it in the list
            insertList((*container)->countries, cInfo);
        }
        else cInfo = getDataNode(findList((*container)->countries, input[3]));
        
        // get the person from the person's hash table
        person = getDataNode(doesKeyExists((*container)->persons, input[0]));

        // if the person does not exist create and insert him in the hashtable
        if(!person)
        {
            // create person
            person = createPerson(input[0], input[1], input[2], cInfo->country, (unsigned short int)atoi(input[4]));

            // insert the person in the hash table
            insertHash((*container)->persons, person, (*container)->persons->hashFunc(getCitizenId(person)));
        }
        
        // temp data to make comparisons, copying the person pointer to avoid unnessecary mallocs
        tempData->person = person;
        
        // get the skip lists associated with the virus
        sList vac = (sList)getEntry((*container)->skip_list_vaccinated, input[5]), notVac = (sList)getEntry((*container)->skip_list_not_vaccinated, input[5]);
        lNode vacNode, notVacNode;
        
        // check if the virus exist and if yes search for the person
        // if not set everything to null
        if (vac) vacNode = skipListSearch(vac, tempData);
        else vacNode = null;

        if (notVac) notVacNode = skipListSearch(notVac, tempData);
        else notVacNode = null;

        // inserstion happens only if it is not a duplicate
        if(comparePersonInfo(person, input[1], input[2], input[3], atoi(input[4]))
        && !(!strcmp(input[6], "NO") && size > 7)   // if the person is not vaccinated then no date is allowed after the answer NO
        && !(!strcmp(input[6], "YES") && size == 7) // if the person is vaccinated then a date is required after the answer YES
        && ((!vac && !notVac) || (!vacNode && !notVacNode))) // if the virus does not exist or if it exists but the person does not exists to a skip list for that virus
        {
            // input stream: id firstname lastname country age virus vaccinationstatus date
            //               0  1         2        3       4   5     6                 7   
            sList skip;
            blm bloom;
            char *virus;

            // check if the given country and virus are already in the right lists, and if there are update encounters variable
            if (!duplicateList((*container)->viruses, input[5])) insertList((*container)->viruses, createString(input[5]));

            // get the virus and country from each list to avoid data duplication, each virus and country are stored in the list
            // every other part of the code gets the pointer of the country or virus
            virus = getDataNode(findList((*container)->viruses, input[5]));

            // update population status
            toUpdate = getDataNode(findList((*container)->countries, input[3]));
            toUpdate->population++;
            
            // update population age group status
            if(atoi(input[4]) <= 20) toUpdate->till20++;
            else if (atoi(input[4]) <= 40) toUpdate->till40++;
            else if (atoi(input[4]) <= 60) toUpdate->till60++;
            else toUpdate->olderThan60++;            

            // check for vaccination status to insert him in the proper skip list and bloom filter
            if (!strcmp(input[6], "YES"))
            {
                // if the citizen id is new create a new skip list and hash entry
                if (!doesKeyExists((*container)->skip_list_vaccinated, virus))
                {
                    // create a new skip list and bloom filter, the given virus did not exist in the app until now
                    skip = createSkipList(virus, destroyPersonVaccination, printPersonVaccinationData, comparePersonVaccinationData);
                    bloom = createFilter(virus, (*container)->bloom_size, hash_i);

                    // insert the data in the two data structs
                    insertSkipList(skip, createPersonVaccinationData(person, true, input[7]));
                    blmInsert(bloom, getCitizenId(person));

                    // insert the two data structs (pointers) in the proper hash tables
                    insertHash((*container)->skip_list_vaccinated, skip, (*container)->skip_list_vaccinated->hashFunc(virus));
                    insertHash((*container)->bloom_filters, bloom, (*container)->bloom_filters->hashFunc(virus));
                }
                else
                {
                    // if not just insert the person in the correct hash entry of the virus
                    insertSkipList(getEntry((*container)->skip_list_vaccinated, virus), createPersonVaccinationData(person, true, input[7]));
                    blmInsert(getEntry((*container)->bloom_filters, virus), getCitizenId(person));
                }
            }
            else
            {
                // if the citizen id is new create a new skip list and hash entry
                if (!doesKeyExists((*container)->skip_list_not_vaccinated, virus))
                {
                    // create only a new skip list for the not vaccinated person and repeat the same steps as above
                    skip = createSkipList(virus, destroyPersonVaccination, printPersonVaccinationData, comparePersonVaccinationData);
                    insertSkipList(skip, createPersonVaccinationData(person, false, null));
                    insertHash((*container)->skip_list_not_vaccinated, skip, (*container)->skip_list_not_vaccinated->hashFunc(virus));
                }
                else
                {
                    // if not just insert the person in the correct hash entry of the virus
                    insertSkipList(getEntry((*container)->skip_list_not_vaccinated, virus), createPersonVaccinationData(person, false, null));
                }
            }
        }
        else
        {
            // update the encountered errors counter
            errors++;
            
            // print the problematic record
            printf("ERROR IN RECORD ");
            for (int i = 0; i < size; i++)
            {
                if (i < size - 1) printf("%s ", input[i]);
                else printf("%s\n", input[i]);
            }
        }

        // deallocate memory used for the string, the parsed string, and the tempdata
        deleteParsedString(input, size);
        free(str);
    }

    printf("ERRORS ENCOUNTERED %d.\n", errors);

    tempData->person = null;

    free(tempData);

    // last deletion of the string if it consumes memory
    if (str) free(str);
    
    return;
}
