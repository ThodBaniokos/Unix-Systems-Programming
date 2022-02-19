// file: skipListIntr.h
// interface of the skip list we're using
#pragma once

// custom libs
#include "Types.h"

// macro defitions
#define MAX_LAYERS 5

// sList
sList createSkipList(char *key, destroyData desFunc, printData printFunc, compareData compareFunc);

// void
void destroySkipList(sList skip);
void printSkipList(sList skip);
void insertSkipList(sList skip, Item data);
void deleteFromSkipList(sList skip, Item data);

// lNode
lNode extractLayerNode(lNode node, int layer);
lNode skipListSearch(sList skip, Item data);

// int
int compareSkipListKey(sList skip, char *key);