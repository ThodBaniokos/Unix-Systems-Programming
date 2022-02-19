// file: linkedList.c
// implementation of the linkedList module functions
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// custom headers below
#include "linkedListIntr.h"

// lList functions
// creates a list with the given compare, print and destroy functions
lList createList(char *key, void *compareFunc, void *printFunc, void *destroyFunc)
{
    // allocates memory for the linked list
    lList newList = malloc(sizeof(*newList));

    // make sure memory was allocated correctly
    assert(newList != null);

    // initialize head and tail to null, size to zero
    newList->head = newList->tail = null;
    newList->size = 0;

    // initialize function pointers
    newList->print = printFunc;
    newList->destroy = destroyFunc;
    newList->compare = compareFunc;

    // set key if there's one
    if (key)
    {
        newList->key = malloc((strlen(key) + 1) * sizeof(char));

        // check for correct memory allocation
        assert(newList->key != null);

        strcpy(newList->key, key);
    }
    else newList->key = null;

    // return the list
    return newList;
}

// lNode functions
lNode findList(Item toCheck, Item toFind)
{
    // iterator to go through the list
    lNode iterator = ((lList)toCheck)->head;

    // loop until the end
    while (iterator != null)
    {
        // if the data is the same between the two nodes return the node
        if(!((lList)toCheck)->compare(iterator->data, toFind))
        {
            return iterator;
        }

        iterator = iterator->next;
    }

    // data to find not in list return null
    return null;
}

// returns the head of the list
lNode getListHead(Item list) { return (list != null) ? ((lList)list)->head : null; }

// returns the tail of the list
lNode getListTail(Item list) { return (list != null) ? ((lList)list)->tail : null; }

// returns the next node of the current one
lNode getListNext(Item node) { return (node != null) ? ((lNode)node)->next : null; }

// returns the node at specified index
lNode getListNodeAt(Item list, int index)
{
    // check if specified index is out of bounds
    if (index > ((lList)list)->size) return null;

    // if index is zero return the head of the list
    if (index == 0) return ((lList)list)->head;

    // iterate through the list to get the specified node
    lNode iterator = ((lList)list)->head;
    for (int i = 0; i < index; i++) iterator = iterator->next;

    return iterator;
}

// Item functions
Item getDataNode(Item toManipulate) { return  (((lNode)toManipulate)) ? ((lNode)toManipulate)->data : null; }

// bool functions
// checks to see if the list is empty
bool isListEmpty(Item toCheck) { return ((((lList)toCheck)->head == null) ? true : false); }

// check for duplicate node
bool duplicateList(Item list, Item toCheck)
{
    // init iterator
    lNode iterator = ((lList)list)->head;

    // iterate through the whole list
    while(iterator != null)
    {
        // if the nodes are equal
        if(!((lList)list)->compare(iterator->data, toCheck))
        {
            // update encounters and return that it exists
            iterator->encounters++;
            return true;
        }

        // continue with the rest of the list
        iterator = iterator->next;
    }

    // return that it does not exist
    return false;
}

// void functions
// inserts new item in the list
void insertList(Item list, Item toInsert)
{
    // creation of new node
    lNode newNode = malloc(sizeof(*newNode));

    // check for proper memory allocation
    assert(newNode != null);

    // initialize variables
    newNode->data = toInsert;
    newNode->encounters = 1;
    newNode->next = null;

    // check if the list is empty
    if (isListEmpty(list))
    {
        ((lList)list)->head = ((lList)list)->tail = newNode;
        ((lList)list)->head->next = ((lList)list)->tail->next = null;
    }
    else
    {
        // insert at the end of the list
        ((lList)list)->tail->next = newNode;
        ((lList)list)->tail = newNode;
    }

    // update size variable
    ((lList)list)->size++;

    return;
}

void insertListSorted(Item list, Item toInsert, secondaryCompare secondaryCmp)
{
    // creation of new node
    lNode newNode = malloc(sizeof(*newNode));

    // check for correct memory allocation
    assert(newNode != null);

    newNode->data = toInsert;
    newNode->next = null;
    newNode->encounters = 1;

    // check for proper memory allocation
    assert(newNode != null);

    // check if the list is empty
    if (isListEmpty(list))
    {
        ((lList)list)->head = ((lList)list)->tail = newNode;
        ((lList)list)->head->next = ((lList)list)->tail->next = null;
        ((lList)list)->size++;
        return;
    }

    // iterate through list to find proper spot for the new node
    lNode iterator = ((lList)list)->head, prev = null;
    while (iterator != null)
    {
        // if the secondary key of the current node is less than
        // the given one, insert properly
        if(secondaryCmp(iterator->data, toInsert) > 0)
        {
            if(iterator == ((lList)list)->head)
            {
                ((lList)list)->head = newNode;
                newNode->next = ((lList)list)->tail = iterator;
            }
            else
            {
                prev->next = newNode;
                newNode->next = iterator;
            }
            ((lList)list)->size++;
            return;
        }
        prev = iterator;
        iterator = iterator->next;
    }

    // insert at the end of the list
    prev->next=newNode;
    ((lList)list)->tail = newNode;
    ((lList)list)->size++;

    return;
}

// insert given item after specified node
void insertListAfterNode(Item list, Item after, Item toInsert)
{
    // creation of new node
    lNode newNode = malloc(sizeof(*newNode));

    // check for correct memory allocation
    assert(newNode != null);

    newNode->data = toInsert;
    newNode->next = null;
    newNode->encounters = 0;

    // check for proper memory allocation
    assert(newNode != null);

    // check if the list is empty
    if (isListEmpty(list))
    {
        ((lList)list)->head = ((lList)list)->tail = newNode;
        ((lList)list)->head->next = ((lList)list)->tail->next = null;
        ((lList)list)->size++;
        return;
    }

    // check if the given node is the tail of the list
    if (((lNode)after) == ((lList)list)->tail)
    {
        // find the previous node and make the insertion
        if (((lNode)after) == ((lList)list)->head)
        {
            ((lList)list)->tail = newNode;
            ((lList)list)->head->next = ((lList)list)->tail;
            return;
        }

        lNode iterator = ((lList)list)->head;
        while (iterator->next != ((lList)list)->tail) iterator = iterator->next;

        // error handler
        if (!iterator) return;

        ((lList)list)->tail = newNode;
        iterator->next->next = ((lList)list)->tail;
        ((lList)list)->size++;
        return;
    }

    // the next node of the new is the specified's next
    // the specifide's next is the new one
    newNode->next = ((lNode)after)->next;
    ((lNode)after)->next = newNode;

    // update size variable
    ((lList)list)->size++;

    return;
}

// insert given item before specified node
void insertListBeforeNode(Item list, Item before, Item toInsert)
{
    // creation of new node
    lNode newNode = malloc(sizeof(*newNode));

    // check for correct memory allocation
    assert(newNode != null);

    newNode->data = toInsert;
    newNode->next = null;
    newNode->encounters = 0;

    // check for proper memory allocation
    assert(newNode != null);

    // check if the list is empty
    if (isListEmpty(list))
    {
        ((lList)list)->head = ((lList)list)->tail = newNode;
        ((lList)list)->head->next = ((lList)list)->tail->next = null;
        ((lList)list)->size++;
        return;
    }

    // check if the node is the head, if true then
    // make the new node the head of the list
    if (((lNode)before) == ((lList)list)->head)
    {
        newNode->next = ((lList)list)->head;
        ((lList)list)->head = newNode;
        ((lList)list)->size++;
        return;
    }

    // find the previous node node and make the insertion
    lNode prev = ((lList)list)->head;
    while (prev->next != ((lNode)before)) prev = prev->next;

    // error handler
    if(!prev) return;

    newNode->next = prev->next;
    prev->next = newNode;

    // update size variable
    ((lList)list)->size++;

    return;
}

// prints the entire list node by node
void printList(Item list)
{
    if (((lList)list)->key) printf("List %s\n", ((lList)list)->key);

    lNode iterator = ((lList)list)->head;
    while(iterator != null)
    {
        ((lList)list)->print(iterator->data);
        iterator = iterator->next;
    }
    return;
}

// delete node from the list
void deleteFromList(Item list, Item node)
{
    // check if the node to delete is the head
    if (((lList)list)->head == ((lNode)node)) ((lList)list)->head = ((lList)list)->head->next;
    else
    {
        // finding the previous node of the one we want to delete
        lNode prev = ((lList)list)->head;
        while (prev->next != ((lNode)node)) prev = prev->next;
        if(prev == null) return;

        // the next node of the prev is the next of the one we want to delete
        prev->next = ((lNode)node)->next;
    }

    // if there's a destroy func the use it and free the memory of the node
    if(((lList)list)->destroy != null) ((lList)list)->destroy(((lNode)node)->data);
    free(node);
    return;
}

// destroys the list
void destroyList(Item toDestroy)
{
    // delete all nodes
    lNode iterator = ((lList)toDestroy)->head, toDelete;
    while(iterator != null)
    {
        toDelete = iterator;
        iterator = iterator->next;
        if(((lList)toDestroy)->destroy != null) ((lList)toDestroy)->destroy(toDelete->data);
        free(toDelete);
    }

    // destroy the list
    if (((lList)toDestroy)->key) free(((lList)toDestroy)->key);
    free(((lList)toDestroy));
    return;
}

// int functions
// retunrs the size of the list
int getListSize(Item list)
{
    // return the size of the list item is not null
    return (list != null) ? ((lList)list)->size : -1; // -1 indicates error
}

// compare the key of the list with the given one
int compareListKey(Item list, char *key)
{
    return strcmp(((lList)list)->key, key);
}

// char functions
// returns the key of the list
char *getListKey(Item list)
{
    return (((lList)list)->key) ? ((lList)list)->key : null;
}