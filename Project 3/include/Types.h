// file: Types.h
// decleration of the types used in this project
#pragma once

// libraries
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>

// macro definitions
#define null NULL
#define FILE_DESCRIPTORS 2

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
    char *key;
    int layerSize;
    lList *layers;
    destroyData destroy;
    printData print;
} *sList;

// struct for the request stats
typedef struct Request_Stats
{
    int total;
    int accepted;
    int rejected;
} req_stats;