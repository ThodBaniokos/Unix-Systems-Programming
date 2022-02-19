// file: ipc_protocolIntr.h
// inter process communication protocol
// include guard
#pragma once

// libs below
#include <stdbool.h>

// message definitions
#define MSG_DIR 1 // send directory
#define MSG_BLM 2 // send bloom filter
#define MSG_CMD_TR_RQ 3 // query travel request
#define MSG_ANS_TR_RQ 4 // answer travel request query
#define MSG_CMD_TR_ST 5 // query travel stats
#define MSG_CMD_ADD_VAC_REC 7 // query add vaccination record
#define MSG_CMD_SCH_VAC_ST 9 // query search vaccination status
#define MSG_ANS_SCH_VAC_ST 10 // answer search vaccination status query
#define MSG_STOP_TRNS 12 // ended message transmision
#define MSG_BUFF_SIZE 13 // send buffer size to monitor process
#define MSG_BLM_SIZE 14 // send bloom filter size to monitor process
#define MSG_AMNT_DIRS 15 // send amount of subdirs each monitor process has
#define MSG_AMNT_BLM 16 // send amount of bloom filters to travel monitor
#define STOP ""

// global functions used by ipc.c
extern int max_int_digits;
extern int max_header_digits;

// function definitions
// int functions
int get_msg_code(char *msg);
int msg_send(int file_desc, int msg_code, char *msg, int buf_size);

// char functions
bool read_specified_bytes(int file_desc, int bytes_to_read, char *msg, int buffer_size, char *buffer);
char *msg_read(int file_desc, int buf_size);
char *get_msg_header(char *msg);

// void functions
void set_max_int_digits(void);
void set_max_header_digits(void);