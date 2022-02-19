// file: mon_commandsIntr.h
// declaration of the monintor commands in the monitor program
#pragma once

// custom headers below
#include "monitorHelpersIntr.h"

// char functions below
char *build_ans(char *ans, int len);
char *travel_req(mon_con *container, char *query, req_stats *stats);
char *search_vac_status(mon_con *container, char *query);

// void functions below
void exitSequence(mon_con *container, char **input_dirs, char **read_files, int amount_of_sub_dirs, int amount_of_files);

// bool functions below
bool valid_vac(char *vac_date, char *travel_date);

// int functions below
int send_ans(struct pollfd *fds, char *ans, int buf_size, int msg_code);
