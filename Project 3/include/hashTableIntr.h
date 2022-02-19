// file: hashTableIntr.h
// contains the declaration of the functions needed
// by the hash table
#pragma once

// libraries below
#include "Types.h"
#include "linkedListIntr.h"

// default size declared here
#define hashDefaultSize 100

// HT functions

// void* -> print function, void* -> delete function, void* -> compare funtion
HT createHash(void *hashFunc, void *printFunc, void *deleteFunc, void *compareFunc);

// bool functions

// HT -> hash table
bool isHashEmpty(Item toCheck);

// Item functions
Item doesKeyExists(Item table, Item keyToCheck);
Item getEntry(Item table, Item entry);

// int functions

// hash function
// Item -> key
int hashFunction(Item key);

// void functions

// HT -> hash table, Item -> to insert
void insertHash(Item toManipulate, Item toInsert, int index);

// HT -> hash table
void printHash(Item toPrint);

void deleteRecordHash(Item table, Item node, Item key);

// HT -> hash table, Item -> to delete
void destroyHash(Item toDelete);
