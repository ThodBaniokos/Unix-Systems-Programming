// file: converters.c
// implementation of the converters used
// libs below
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// custom headers below
#include "blmIntr.h"
#include "utilsIntr.h"
#include "hashFunctions.h"
#include "convertersIntr.h"

// converts given int to string
char *int_to_string(const int num)
{
    char *converted;

    // get the right amount of chars needed to store the given int
    int len = 0;
    len = snprintf(NULL, len, "%d", num);

    // allocate memory
    converted = calloc((len + 1), sizeof(char));

    // check for correct memory allocation
    assert(converted != null);

    // convert given int to string
    snprintf(converted, len + 1, "%d", num);

    // set null terminator to the newly created string
    converted[len] = '\0';

    // return the string
    return converted;
}

// converts a given bloom filter to string
char *blm_to_string(const blm to_convert, int blm_size)
{
    // check if the given bloom filter is not null
    assert(to_convert != null);

    // calculate the needed length of chars to convert the bit array ints to strings
    int counter = 0;

    for (int i = 0; i < to_convert->blmSize; i++)
    {
        // calculate the length of chars needed for the i'th integer in the bit array
        int len = 0;
        len = snprintf(NULL, len, "%u", to_convert->bitarray[i]);

        // update length counter
        counter += (len + 1);
    }


    // allocate memory for the string representation of the bitarray
    // 2 * counter, 1 number and a space
    char *bitarray = calloc((2 * counter) + 1, sizeof(char));

    // check for correct memory allocation
    assert(bitarray != null);

    // convert every int in the bitarray to string
    int index = 0;

    for(int i = 0; i < to_convert->blmSize; i++)
    {
        index += snprintf(&bitarray[index], counter, "%u ", to_convert->bitarray[i]);
    }

    // calculate the needed length for the whole conversion, each field is seperated with a comma, helps in the decoding phase
    int len = 0;

    len = snprintf(NULL, len, "%d,%s,%d,%s,%zu", blm_size, to_convert->key, to_convert->items, bitarray, to_convert->bitAmount);

    // allocate memory for the whole conversion
    char *converted = calloc((len + 1), sizeof(char));

    // check for correct memory allocation
    assert(converted != null);

    // convert the bloom filert to a string
    snprintf(converted, len + 1, "%d,%s,%d,%s,%zu", blm_size, to_convert->key, to_convert->items, bitarray, to_convert->bitAmount);

    // free allocated memory for the bitarray string
    free(bitarray);

    // return the converted bloom filter
    return converted;
}

// converts a given string to bloom filter
blm string_to_blm(char *bloom_str)
{
    // check if the given string is not null
    assert(bloom_str != null);

    // size of the parsed string
    int size;
    char **bloom_parsed;

    // parse the given string with the custom string parser, the seperator is set by us as a comma
    bloom_parsed = stringParser(bloom_str, &size, ",");

    // create a new bloom filter with the converted key, size and the given hash function from instructors
    blm converted = createFilter(bloom_parsed[1], atoi(bloom_parsed[0]), hash_i);

    // copy the converted fields to the newly created bloom filter
    converted->items = atoi(bloom_parsed[2]);
    converted->bitAmount = atoi(bloom_parsed[4]);

    // use strtok to get the bitarray numbers, the separator is a space (" ")
    char *num = strtok(bloom_parsed[3], " ");

    // convert every number and copy it to the bitarray of the new bloom filter
    for(int i = 0; i < converted->blmSize; i++)
    {
        converted->bitarray[i] = atoi(num);
        num = strtok(NULL, " ");
    }

    // delete allocated memory for the parsed bloom filter string
    deleteParsedString(bloom_parsed, size);

    // return the converted bloom filter
    return converted;
}
