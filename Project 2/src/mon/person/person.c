// file: person.c
// imlementation of the util funcs for the person struct
// libs
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// custom libs
#include "personIntr.h"

// macro defitions
#define null NULL

// pInfo
// creates a new object of the struct person
pInfo createPerson(char *id, char *firstName, char *lastName, char *country, unsigned short int age)
{
    // allocate memory for struct
    pInfo person = malloc(sizeof(*person));

    // error handler
    assert(person != null);

    // create string needed by the struct
    person->citizenID = malloc((strlen(id) + 1) * sizeof(char)); strcpy((char *)person->citizenID, id);
    person->firstName = malloc((strlen(firstName) + 1) * sizeof(char)); strcpy((char *)person->firstName, firstName);
    person->lastName = malloc((strlen(lastName) + 1) * sizeof(char)); strcpy((char *)person->lastName, lastName);
    person->country = country;

    // add age of the person
    person->age = age;

    return person;
}


// unsigned char
// returns the citizen id of the person
char *getCitizenId(pInfo person)
{
    return person->citizenID;
}

// returns the first name of the person
char *getFirstName(pInfo person)
{
    return person->firstName;
}

// returns the last name of the person
char *getLastName(pInfo person)
{
    return person->lastName;
}

// returns the country of the person
char *getCountry(pInfo person)
{
    return person->country;
}

// unsigned short int
// returns the age of the person
unsigned short int getAge(pInfo person)
{
    return person->age;
}

// int
// compares the id's of the two persons
int compareCitizenId(Item citizen, Item id)
{
    return strcmp(((pInfo)citizen)->citizenID, (char *)id);
}

// void
// prints the person information
void printPersonInformation(Item person)
{
    printf("%s %s %s %d\n", ((pInfo)person)->citizenID, ((pInfo)person)->firstName, ((pInfo)person)->lastName, ((pInfo)person)->age);
    return;
}

// deletes the allocated memory for the person object
void deletePerson(Item person)
{
    free(((pInfo)person)->citizenID);
    free(((pInfo)person)->firstName);
    free(((pInfo)person)->lastName);
    free(person);
    return;
}
