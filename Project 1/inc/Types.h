// file: Types.h
// decleration of the types used in this project
#pragma once

// libraries
#include <stdbool.h>
#include <assert.h>

// macro definitions
#define null NULL
#define ERROR -1
#define EXIT 0
#define SUCCESS 1

// generic item type to avoid multiple implementations of the same structs, i.e. linked list
typedef void *Item;

// pointers to functions
typedef void (*printData)(Item data);
typedef void (*destroyData)(Item data);
typedef int (*hashData)(Item data); // for the hash table
typedef unsigned long (*bloom_hash)(unsigned char *str, unsigned int i);
typedef int (*compareData)(Item data1, Item data2);
typedef int (*secondaryCompare)(Item data1, Item data2);

// struct of the generic list node we're going to use
typedef struct Node
{
    Item data;
    int encounters;
    struct Node *next;
} *lNode;

// struct of the generic list we're going to use
typedef struct LinkedList
{
    int size;
    char *key;
    lNode head;
    lNode tail;
    printData print;
    destroyData destroy;
    compareData compare;
} *lList;

// struct of the hash table we're going to use
typedef struct HashTable
{
    int size;
    hashData hashFunc;
    lList *tableList;
} *HT;

// struct for the bloom filter
typedef struct BloomFilter
{
    char *key;
    int items;
    int blmSize; // in bytes
    unsigned int *bitarray;
    size_t bitAmount; // amount of bits in an unsigned int var
    bloom_hash hashFunc;
} *blm;

// struct for the skip list
typedef struct SkipList
{
    char * key;
    int layerSize;
    lList *layers;
    destroyData destroy;
    printData print;
} *sList;
