// file: answerIntr.h
// interface of the answer, to the main process, functions used
// include guard
#pragma once

// libs below
#include <poll.h>

// custom headers below
#include "Types.h"

// char functions below
char *build_ans(char *str, int len);

// int functions below
int send_ans(struct pollfd *fds, char *ans, int buf_size, int msg_code);