// file: blmIntr.h
// interface of the bloom filter we're using
#pragma once

// custom libs
#include "Types.h"

// macro defitions
#define hashFucntions 16
#define bitsPerByte 8

// void 
void initBits(blm filter);
void setBit(blm filter, int bitNum);
void clearBit(blm filter, int bitNum);
void blmInsert(blm filter, Item data);
void blmDestroy(blm discard);

// blm
blm createFilter(char *key, int size, void *hashFunc);

// bool
bool blmLookup(blm filter, Item data);

// int
int getBit(blm filter, int bitNum);
int compareBloomKey(blm filter, char *key);