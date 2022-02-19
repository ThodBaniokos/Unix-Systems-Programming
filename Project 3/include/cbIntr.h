// file: cbIntr.h
// include guard
#pragma once

// headers below
#include <pthread.h>

// declaration of the circural buffer
typedef struct CircuralBuffer
{
    char **file_paths;
    int start;
    int end;
    int item_count;
    int cb_size;
} *cb;

// pthread conditional variables
extern pthread_cond_t buffer_not_full;
extern pthread_cond_t buffer_not_empty;
extern pthread_mutex_t init_mtx;

// size of the strings array and the strings array itself
extern char **input_dir_paths;
extern int input_dir_paths_size;
extern int paths_left;

// functions declerations
// cb functions
cb cb_init(int size, int input_dirs_size, char **paths);

// char functions
char *cb_get(cb circ_buf);

// void functions
void cb_add(cb circ_buf, char *path);
void cb_destroy(cb discard);
void init_cond_mtx(void);
void des_cond_mtx(void);