// file: cb.c
// implementation of the circural buffer used in the project
// this implementation is based on the K24 - Unix Systems Programming course
// posted here: http://cgi.di.uoa.gr/~antoulas/k24/lectures/l13.pdf
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

// custom headers below
#include "cbIntr.h"

// globaly used mutex for thread synchronization
pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

// globaly used conditional variables for thread synchronization
pthread_cond_t buffer_not_full;
pthread_cond_t buffer_not_empty;

// size of the strings array and the strings array itself
int input_dir_paths_size;
int paths_left;

// cb functions below
// initializes the circural buffer
cb cb_init(int size, int input_dirs_size, char **paths)
{
    // allocate memory for the circural buffer
    cb new_buffer = malloc(sizeof(*new_buffer));

    // check for correct memory allocation
    assert(new_buffer != NULL);

    // allocate memory for the array of strings, representing the paths of the files
    new_buffer->file_paths = calloc(size, sizeof(char *));

    // check for correct memory allocation
    assert(new_buffer->file_paths != NULL);

    // set the fields of the buffer to their initial state
    new_buffer->start = 0;
    new_buffer->item_count = 0;
    new_buffer->end = -1;
    new_buffer->cb_size = size;

    // return the new buffer
    return new_buffer;
}

// char functions
// gets an item from the circural buffer if it's not empty
char *cb_get(cb circ_buf)
{
    char *file_path = NULL;

    // lock the mutex to avoid possible data race from threads
    pthread_mutex_lock(&mtx);

    // non empty buffer
    while (circ_buf->item_count <= 0)
    {
        pthread_cond_wait(&buffer_not_empty, &mtx);
    }

    // get the file path from the start of the buffer
    file_path = circ_buf->file_paths[circ_buf->start];
    circ_buf->start = (circ_buf->start + 1) % circ_buf->cb_size;
    circ_buf->item_count--;

    pthread_mutex_unlock(&mtx);

    // return the path
    return file_path;
}

// void functions
// adds an item in the buffer if there's an empty spot
void cb_add(cb circ_buf, char *path)
{
    // lock the mutex to avoid possible data race from threads
    pthread_mutex_lock(&mtx);

    // empty spot in spot
    while (circ_buf->item_count >= circ_buf->cb_size)
    {
        pthread_cond_wait(&buffer_not_full, &mtx);
    }

    // add a new file path to the end of the buffer
    circ_buf->end = (circ_buf->end + 1) % circ_buf->cb_size;
    circ_buf->file_paths[circ_buf->end] = path;
    circ_buf->item_count++;

    pthread_mutex_unlock(&mtx);

    return;
}

// delete allocated memory of the circural buffer
void cb_destroy(cb discard)
{
    if(discard->file_paths) free(discard->file_paths);
    if(discard) free(discard);
}

// initializes the mutex and conditional variables
void init_cond_mtx(void)
{
    pthread_mutex_init(&mtx, NULL);
    pthread_cond_init(&buffer_not_full, NULL);
    pthread_cond_init(&buffer_not_empty, NULL);
    return;
}

// delete conditional variables
void des_cond_mtx(void)
{
    pthread_mutex_destroy(&mtx);
    pthread_cond_destroy(&buffer_not_full);
    pthread_cond_destroy(&buffer_not_empty);
    return;
}
