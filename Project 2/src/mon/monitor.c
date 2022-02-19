// file: monitor.c
// implementation of the main function of the monitor process
// libraries below
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// custom headers below
#include "loggerIntr.h"
#include "ipc_protocolIntr.h"
#include "mon_commandsIntr.h"
#include "monitorHelpersIntr.h"
#include "mon_sig_actionsIntr.h"

// path of the logs directory
#define LOGSPATH "./logs/"

// execl("./Monitor", "./Monitor", proc_info[i].write_np, proc_info[i].read_np, NULL);
// ./Monitor ./tmp/main_proc_write_n ./tmp/child_proc_write_n

int main(int argc, char *argv[])
{
    // init signal handlers and ipc protocol variables
    mon_general_initialization();

    char *read_p, *write_p;

    if (argc_argv_manipulator_mon(argc, argv, &read_p, &write_p))
    {
        perror("Cannot read command line agruments");
        return EXIT_FAILURE;
    }

    mon_con container;
    char *buffer, **input_dirs, **read_files;
    struct pollfd files_descs[FILE_DESCRIPTORS];
    int read_fd, write_fd, buffer_size = 0, blm_size = 0, amount_of_sub_dirs, amount_of_files;

    // request received in the monitor process
    req_stats stats;

    // initialize the statistics
    stats.accepted = stats.rejected = stats.total = 0;

    // initialize every field of the monitor process and send the bloom filters to the main process
    monitor_initialization(write_p, read_p, &write_fd, &read_fd, files_descs, &container, &input_dirs, &read_files, &amount_of_sub_dirs, &amount_of_files, &buffer_size, &blm_size);

    // ready to receive queries
    while (true)
    {
        bool transmission_flag = false;
        char *ans = null;
        int msg_code = -1;

        // check if the program received a signal
        // received sigusr1
        if (received_sigusr1)
        {
            if (updateDataStructs(&container, input_dirs, amount_of_sub_dirs, read_files, amount_of_files) == EXIT_FAILURE) exit(EXIT_FAILURE);
            sendBlooms(&container, files_descs, buffer_size, blm_size);
            received_sigusr1 = false;
        }

        // received sigint
        if (received_sigint)
        {
            produce_log_mon(container, &stats, LOGSPATH);
        }

        // received sigquit
        if (received_sigquit)
        {
            produce_log_mon(container, &stats, LOGSPATH);
            exitSequence(&container, input_dirs, read_files, amount_of_sub_dirs, amount_of_files);
            break;
        }

        // starting loop to receive a message from the main process
        while(!transmission_flag)
        {
            // if the program received a signal break from this loop
            if (received_sigusr1 || received_sigint || received_sigquit) break;

            // poll syscall to get ready file descriptors
            int ready_fds = poll(files_descs, FILE_DESCRIPTORS, 0);

            // error checking
            if (ready_fds == -1)
            {
                perror("poll at monitor.c");
                exit(EXIT_FAILURE);
            }
            else if (ready_fds == 0) continue;

            // iterate through the file descriptors
            for(int i = 0; i < FILE_DESCRIPTORS; i++)
            {
                // check if the returned event of the file descriptor is POLLIN, i.e. there is data to read
                if ((files_descs[i].revents & POLLIN) == POLLIN)
                {
                    // receive the message
                    if ((buffer = msg_read(files_descs[i].fd, buffer_size)) == NULL)
                    {
                        perror("msg_read at monitor.c");
                        exit(EXIT_FAILURE);
                    }

                    // switch case to decode the message and start executing the given query
                    switch (get_msg_code(buffer))
                    {
                        // travel request query
                        case MSG_CMD_TR_RQ:

                            // get answer
                            ans = travel_req(&container, buffer + max_header_digits, &stats);

                            // set the message code of the answer to message answert travel request
                            msg_code = MSG_ANS_TR_RQ;
                            break;
                        case MSG_CMD_SCH_VAC_ST:

                            // search vaccination status query
                            ans = search_vac_status(&container, buffer + max_header_digits);

                            // if answer is null, no person with given citizenID exists, sending stop immediately
                            // else set message code to message answer search vaccination status
                            if (!ans) msg_code = MSG_STOP_TRNS;
                            else msg_code = MSG_ANS_SCH_VAC_ST;
                            break;
                        case MSG_STOP_TRNS:

                            // received stop transmision
                            transmission_flag = true;
                            break;
                    }

                    // free allocatated memory for buffer if exists
                    if (buffer) free(buffer);
                }
                else continue;
            }
        }

        // if msg_code is not set then just dont send anythin
        if (msg_code != -1) send_ans(files_descs, ans, buffer_size, msg_code);
    }

    exit(EXIT_SUCCESS);
}