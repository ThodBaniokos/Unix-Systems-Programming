// file: blm.c
// implementation of the bloom filter
// c libs
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// custom libs
#include "blmIntr.h"
#include "commonIntr.h"

// void 
// init all bits of the bit array with 0
void initBits(blm filter)
{
    // iterate through all the bits in the bit array to set them the initial value of 0
    for(int i = 0; i < filter->blmSize * filter->bitAmount; i++)
    {
        // find the index of the i'th bit with i / filter->bitAmount, i.e. if i = 1 and bitAmount = 32 then we are in the first cell of the array etc.
        // get the specific bit of the index with i % filter->bitAmount and set it to 0
        // &= 0 -> bitwise and with 0 => the bit in the number is going to be 0
        filter->bitarray[i / filter->bitAmount] &= 0 << (i % filter->bitAmount);
    }

    return;
}

// sets the specified bit, from bitNum, in the bit array to 1
void setBit(blm filter, int bitNum)
{
    // find the index of the i'th bit with bitNum / filter->bitAmount, i.e. if i = 1 and bitAmount = 32 then we are in the first cell of the array etc.
    // get the specific bit of the index with bitNum % filter->bitAmount
    // |= 1 -> bitwise or with 1 => the bit in the number is going to be 1
    filter->bitarray[bitNum / filter->bitAmount] |= 1 << (bitNum % filter->bitAmount);

    return;
}

// sets the specified bit, from bitNum, in the bit array to 0
void clearBit(blm filter, int bitNum)
{
    // find the index of the i'th bit with bitNum / filter->bitAmount, i.e. if i = 1 and bitAmount = 32 then we are in the first cell of the array etc.
    // get the specific bit of the index with bitNum % filter->bitAmount
    // &= 0 -> bitwise and with 0 => the bit in the number is going to be 0
    filter->bitarray[bitNum / filter->bitAmount] &= 0 << (bitNum % filter->bitAmount);

    return;
}

// inserts the data in the filter, ie changing the bits
void blmInsert(blm filter, Item data)
{
    // we iterate through the amount of hash functions we have and we're setting each bit to one
    for(int i = 0; i < hashFucntions; i++)
    {
        setBit(filter, filter->hashFunc(((unsigned char *)data), i) % (filter->blmSize * filter->bitAmount));
    }

    // update the filter items variable, represents the items inserted
    filter->items++;

    return;
}

// free allocated memory for blm
void blmDestroy(blm discard)
{
    // free allocated memory of the bit array first
    free(discard->bitarray);

    // free allocated memory of the key if exists
    if(discard->key) free(discard->key);

    // free memory of struct
    free(discard);

    return;
}

// blm
blm createFilter(char *key, int size, void *hashFunc)
{
    // allocate memory for the blm
    blm newFilter = malloc(sizeof(*newFilter));

    // error handler
    assert(newFilter != null);

    // set the identifier of the bloom filter, if exists
    if (key != null) { newFilter->key = malloc((strlen(key) + 1) * sizeof(char)); strcpy(newFilter->key, key); }
    else newFilter->key = null;

    // allocate memory for the bit array
    // the appropriate amount is calculated by the given size, *IN BYTES*, divided by the amount 
    // of memory needed for unsigned ints, used them beacause there's no bit for a sign
    // we need only bits so the actual size is the given size / sizeof(unsigned int) multiplied
    // by the sizeof(unsigned int) in order to get the bits we want
    newFilter->bitarray = malloc((size / sizeof(unsigned int)) * sizeof(unsigned int));

    // keep needed values
    newFilter->bitAmount = sizeof(unsigned int) * bitsPerByte;
    newFilter->blmSize = size / sizeof(unsigned int);
    newFilter->hashFunc = hashFunc;
    newFilter->items = 0;

    // init the filter bit array
    initBits(newFilter);

    return newFilter;
}

// bool
// checks if the given data is propably stored
bool blmLookup(blm filter, Item data)
{
    // we iterate through the amount of hash functions we have and we're taking the bit that
    // it returns if it is zero then the given data is not in the filter and we return false
    for(int i = 0; i < hashFucntions; i++)
    {
        if (getBit(filter, filter->hashFunc((data), i) % (filter->blmSize * filter->bitAmount)) == 0)
        {
            return false;
        }
    }

    // if all the bits are 1 then the data is propably in the filter, return true
    return true;
}


// int
// returns the specified bit, from bitNum
int getBit(blm filter, int bitNum)
{
    // find the index of the i'th bit with bitNum / filter->bitAmount, i.e. if i = 1 and bitAmount = 32 then we are in the first cell of the array etc.
    // get the specific bit of the index with bitNum % filter->bitAmount
    return (filter->bitarray[bitNum / filter->bitAmount] & (1 << (bitNum % filter->bitAmount)));
}

// compares the key of the bloom filter with the given one
int compareBloomKey(blm filter, char *key)
{
    return strcmp(filter->key, key);
}
