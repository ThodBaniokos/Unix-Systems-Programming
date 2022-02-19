// file: hashTable.c
// implementation of the functions needed
// for the hash table to work properly
// libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// custom headers below
#include "hashTableIntr.h"

// HT functions
// creates an empty hash table
HT createHash(void *hashFunc, void *printFunc, void *deleteFunc, void *compareFunc)
{
    // Allocate memory for the new hash table
    HT table = malloc(sizeof(*table));

    // check for proper memory allocation
    assert(table != null);

    // initialize variables
    table->size = 0;
    table->hashFunc = hashFunc;

    // create the linked lists
    table->tableList = malloc(hashDefaultSize * sizeof(*table->tableList));

    // check for correct memory allocation
    assert(table->tableList != null);

    // initialize list heads
    for(int i = 0; i < hashDefaultSize; i++)
    {
        table->tableList[i] = createList(null, compareFunc, printFunc, deleteFunc);

        // check for correct memory allocation
        assert(table->tableList[i] != null);
    }

    // return the newly created hash table
    return table;
}

// bool functions
// checks if the hash table is empty or not
bool isHashEmpty(Item toCheck) { return ((((HT)toCheck)->size == 0) ? true : false); }

// Item functions
Item doesKeyExists(Item table, Item keyToCheck) { return findList(((HT)table)->tableList[((HT)table)->hashFunc(keyToCheck)], keyToCheck); }

// returns the entry
Item getEntry(Item table, Item entry)
{
    lNode temp = doesKeyExists(table, entry);

    return (temp) ? getDataNode(temp) : null;
}

// int functions
// hashes the given key to generate an index for the hash table
int hashFunction(Item key)
{
	int h=0, a=127;
	for(; *(char *)key != '\0'; key++) h = (a*h + *(char *)key) % hashDefaultSize;
	return h;
}

// void functionskey
// inserts a new item in the hash
void insertHash(Item toManipulate, Item toInsert, int index)
{
    // insert given data to list indicated by index
    insertList(((HT)toManipulate)->tableList[index], toInsert);

    // update size counter
    ((HT)toManipulate)->size++;
    return;
}

// prints the content of the hash
void printHash(Item toPrint)
{
    // prints the content of all the lists inside the table
    for(int i = 0; i < hashDefaultSize; i++) printList(((HT)toPrint)->tableList[i]);
}

// deletes the record with the given key from the hash table
void deleteRecordHash(Item table, Item node, Item key)
{
    // delete a record from the hash indicated by the hashed key
    deleteFromList(((HT)table)->tableList[((HT)table)->hashFunc(key)], node);

    // update size counter
    ((HT)table)->size--;
    return;
}

// deletes the entire hash
void destroyHash(Item toDelete)
{
    // deallocating each pointer to list
    for(int i = 0; i < hashDefaultSize; i++) destroyList(((HT)toDelete)->tableList[i]);

    // deallocating the main pointer
    free(((HT)toDelete)->tableList);

    // deallocating the hash
    free(toDelete);
    return;
}