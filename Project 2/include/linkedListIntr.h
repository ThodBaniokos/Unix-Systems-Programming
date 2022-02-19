// file: linkedListIntr.h
// contains the declarations of the functions needed
// for the implementation of the generic linked list
// we're going to use
#pragma once

// libraries below
#include "Types.h"

// declarations below
// lList functions
lList createList(char *key, void *compareFunc, void *printFunc, void *destroyFunc);

// lNode functions
lNode findList(Item toCheck, Item toFind);
lNode getListHead(Item list);
lNode getListTail(Item list);
lNode getListNext(Item node);
lNode getListNodeAt(Item list, int index);

// Item functions
Item getDataNode(Item toManipulate);

// bool functions
bool isListEmpty(Item toCheck);
bool duplicateList(Item list, Item toCheck);

// void functions
void insertList(Item list, Item toInsert);
void insertListSorted(Item list, Item toInsert, secondaryCompare secondaryCmp);
void insertListAfterNode(Item list, Item after, Item toInsert);
void insertListBeforeNode(Item list, Item before, Item toInsert);
void printList(Item list);
void deleteFromList(Item list, Item toDelete);
void destroyList(Item toDestroy);

// int functions
int getListSize(Item list);
int compareListKey(Item list, char *key);

// char functions
char *getListKey(Item list);