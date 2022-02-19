// file: skipList.c
// implementation of the skip list
// c libs
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// custom libs
#include "linkedListIntr.h"
#include "skipListIntr.h"

// sList
sList createSkipList(char *key, destroyData desFunc, printData printFunc, compareData compareFunc)
{
    // allocate memory for the new skip list
    sList skip = malloc(sizeof(*skip));

    // error handler
    assert(skip != null);

    // set the identifier of the skip list
    if (key != null)
    {
        // country is an external stored variable, no need to allocate new memory, or delete it afterwards it will be deleted elsewhere
        skip->key = key;
    }
    else skip->key = null;


    // get the height of the skip list
    skip->layerSize = MAX_LAYERS;

    // allocate memory for the lists
    skip->layers = malloc(skip->layerSize * sizeof(*skip->layers));

    // create all the other layers but skip layer 0
    for(int i = 0; i < skip->layerSize; i++)
    {
        // we only need sto store the pointer to the print function
        if (!i) skip->layers[i] = createList(null, compareFunc, printFunc, desFunc);
        else skip->layers[i] = createList(null, null, null, null);
    }

    return skip;
}

// void 
// frees up memory of the skip list
void destroySkipList(sList skip)
{
    // destroy all the layers from top to bottom layer
    for(int i = skip->layerSize - 1; i >= 0; i--) destroyList(skip->layers[i]);

    // free memory of skip list struct
    free(skip->layers);
    free(skip);

    return;
}

// prints the skip list
void printSkipList(sList skip)
{
    // print skip list key if exists
    if (skip->key) printf("Skip List %s\n", skip->key);

    // print every layer of the skip list
    for(int i = 0; i < skip->layerSize; i++)
    {
        printf("Layer %d\n", i);
        lNode node = getListHead(skip->layers[i]);
        while (node != null)
        {
            skip->layers[0]->print(getDataNode(extractLayerNode(node, i)));
            node = getListNext(node);
        }
    }

    return;
}

// inserts a new element in the skip list, while preserving the order of the list
void insertSkipList(sList skip, Item data)
{
    // if the base layer is empty all the layers are empty, insert the node in every layer
    // because it represents the sentinel of all the operations
    if (isListEmpty(skip->layers[0]))
    {
        // insert in the first layer the actual data
        insertList(skip->layers[0], data);
        
        // insert in all the other layers the node of the previous layer
        for (int i = 1; i < skip->layerSize; i++) insertList(skip->layers[i], getListHead(skip->layers[i - 1]));
        return;
    }

    // variables to store current and previous node
    lNode currNode = getListHead(skip->layers[skip->layerSize - 1]), prevNode = null;
    
    // variables to store the extracted, from layer, node and the path
    lNode extracted, path[skip->layerSize];

    // start from top layer
    int currLayer = skip->layerSize - 1;

    while (currLayer >= 0)
    {
        // extract the node for the current to the base layer
        extracted = extractLayerNode(currNode, currLayer);

        // data already in the list, return nothig and notify user
        if (skip->layers[0]->compare(getDataNode(extracted), data) == 0)
        {
            printf("Node Data : "); skip->layers[0]->print(getDataNode(extracted));
            printf("Given Data : "); skip->layers[0]->print(data);
            printf("Given data already in the skip list, aborting...\n");
            free(data);
            return;
        }
        else if (skip->layers[0]->compare(getDataNode(extracted), data) > 0)
        {
            // data is before this node, we're going down one layer so get the appropriate nodes
            // if the extracted node is the head of the tail we have to put the new node in that position
            // we're using null to make the insertion before the head possible, i.e. null marks no path -> before the head
            // if not then because we're going to change layers we're storing the position of the previous
            // node to store the new one after it
            if (extracted == getListHead(skip->layers[currLayer])) path[currLayer] = null;
            else path[currLayer] = prevNode;

            // if we're at the base layer stopping, because we've found the right path
            if (!currLayer) break;

            // continue with the layer below this one and start from the head of the list
            currNode = getListHead(skip->layers[--currLayer]);
        }
        else if (skip->layers[0]->compare(getDataNode(extracted), data) < 0)
        {
            // data is after this node or we've reached the end of the current layer,
            // if we're at the tail of the list then we have to store the new node at this layer
            // in the end, we're storing the current node at the path array because the new node will be
            // the tail of this list
            if (!getListNext(currNode)) 
            {
                path[currLayer] = currNode;

                // if the current layer is the base one stop because we've found the correct path
                // for the insertion
                if (!currLayer) break;

                // if not then continue with the layer below and start from this node to the other ones, we found a lower bound
                currNode = getDataNode(currNode);
                currLayer--;
            }
            else
            {
                // if the current node is not the tail then we're storing the current node as
                // the previous one and we're continuing with the rest of the current list to
                // find the correct node in this layer
                prevNode = currNode;
                currNode = getListNext(currNode);
            }
        }
    }

    // the new node has to be inserted in the base layer
    if (!path[0]) insertListBeforeNode(skip->layers[0], getListHead(skip->layers[0]), data);
    else insertListAfterNode(skip->layers[0], path[0], data);

    // for each layer we're deciding, via the rand function, if we're going to insert the node
    // if not then stop the insertion proccess but if the node is the
    // head, then insert it in every layer
    for(int i = 1; i < skip->layerSize; i++)
    {
        if (!path[i]) insertListBeforeNode(skip->layers[i], getListHead(skip->layers[i]), getListHead(skip->layers[i - 1]));
        else if ((rand() * 1.0) / RAND_MAX > 0.49) insertListAfterNode(skip->layers[i], path[i], getListNext(path[i - 1]));
        else break;
    }

    return;
}

// deletes an element from the skip list while preserving the order of the list
void deleteFromSkipList(sList skip, Item data)
{
    // bool to determine if a deletion happend
    bool haveDeleted = false;

    // variable to store the node itself
    lNode currNode = getListHead(skip->layers[skip->layerSize - 1]), extracted;

    // start from top layer
    int currLayer = skip->layerSize - 1;
    
    while (currLayer >= 0)
    {
        // extract the actual data from the layer
        extracted = extractLayerNode(currNode, currLayer);
        
        if (skip->layers[0]->compare(getDataNode(extracted), data) == 0)
        {
            // found the node delete it
            haveDeleted = true;
            deleteFromList(skip->layers[currLayer], currNode);

            // continue with the rest of the layers or stop because we're at the base level
            if (--currLayer < 0) break;
            currNode = getListHead(skip->layers[currLayer]);
            continue;

        }
        else if (skip->layers[0]->compare(getDataNode(extracted), data) > 0)
        {
            // data is before this node, we're going down one layer so get the appropriate nodes
            if (--currLayer < 0) break;
            currNode = getListHead(skip->layers[currLayer]);
        }
        else if (skip->layers[0]->compare(getDataNode(extracted), data) < 0)
        {
            // data is after this node or we've reached the end of the current layer,
            // continue with the rest of the current layer, or go to the layer below
            if (!getListNext(currNode))
            {
                // update the layer variable in order for the extracted node at line 195 will be the same data
                // of the layer below, we've found a lower bound, only checking above that
                if (currLayer) currLayer--;
            }
            else currNode = getListNext(currNode);
        }
    }

    if (!haveDeleted) printf("Non existing data...\n");

    return;
}

// lNode
// gets the node of the specified layer
lNode extractLayerNode(lNode node, int layer)
{
    // error handler
    assert(layer >= 0);

    // set the given node to an auxiliary variable
    lNode temp = node;

    // call the get data node, which returns the data stored in the node
    // because the data of each node, except the base layer, is a node too
    // if we want to get the actual data we have to get the data of the node
    // layer times in odrer to go to the base layer and find the actual data stored
    // if the layer is 0 then we just return the node because we're in the base layer
    for(int i = 0; i < layer; i++) temp = getDataNode(temp);

    // return the node that the given one is pointing in the base layer
    return temp;
}

// returns the node of the base layer if the given data is found
lNode skipListSearch(sList skip, Item data)
{
    // variable to store the node itself
    lNode currNode = getListHead(skip->layers[skip->layerSize - 1]);
    lNode extracted;

    // start from top layer
    int currLayer = skip->layerSize - 1;
    
    while (currLayer >= 0)
    {
        extracted = extractLayerNode(currNode, currLayer);

        if (skip->layers[0]->compare(getDataNode(extracted), data) == 0)
        {
            return extracted;
        }
        else if (skip->layers[0]->compare(getDataNode(extracted), data) > 0)
        {
            // data is before this node, we're going down one layer so get the appropriate nodes
            if (--currLayer < 0) break;
            currNode = getListHead(skip->layers[currLayer]);
        }
        else if (skip->layers[0]->compare(getDataNode(extracted), data) < 0)
        {
            // data is after this node or we've reached the end of the current layer,
            // continue with the rest of the current layer, or go to the layer below
            if (!getListNext(currNode))
            {
                // update the layer variable in order for the extracted node at line 195 will be the same data
                // of the layer below, we've found a lower bound, only checking above that
                if (currLayer) 
                {
                    currNode = getDataNode(currNode);
                    currLayer--;
                }
                else break;
            }
            else currNode = getListNext(currNode);
        }
    }

    return null;
}

// int
// compares the key of the skip list
int compareSkipListKey(sList skip, char *key)
{
    return strcmp(skip->key, key);
}
