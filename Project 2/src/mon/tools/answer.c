// file: answer.c
// implementation of the answer, to the main process, related functions
// libs below
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// custom libs below
#include "answerIntr.h"
#include "ipc_protocolIntr.h"

// builds answer
char *build_ans(char *str, int len)
{
    // allocate memory for the given string, based on the given length
    char *ans = calloc(len + 1, sizeof(char));

    // check for correct memory allocation
    assert(ans != null);

    // copy the contents of the given str to the answer
    strcpy(ans, str);

    // return the answer
    return ans;
}

// sends answer to main process
int send_ans(struct pollfd *fds, char *ans, int buf_size, int msg_code)
{
    int transmission_index = 0;
    bool transmission_flag = false;

    // if the message code is to stop transmission then update the transmission index
    if (msg_code == MSG_STOP_TRNS) transmission_index++;

    // loop until message with code stop sent
    while (!transmission_flag)
    {
        // poll() syscall to find ready file descriptors
        int ready_fds = poll(fds, FILE_DESCRIPTORS, 0);

        // error checking
        if (ready_fds == -1)
        {
            perror("poll at monitor.c");
            exit(EXIT_FAILURE);
        }
        else if (ready_fds == 0) continue;

        // loop through the file descriptors store in the pollfd struct of the monitor process
        for(int i = 0; i < FILE_DESCRIPTORS; i++)
        {
            // check if the events returned from poll() is POLLOUT, i.e. writing will not block
            if ((fds[i].revents & POLLOUT) == POLLOUT)
            {
                // if the transmision index is 0 then send the answer first
                // if it is 1 then send stop transmission message
                if(transmission_index == 0)
                {
                    // error checking for empty answer
                    if (!ans)
                    {
                        transmission_index++;
                        break;
                    }

                    // send answer
                    if (msg_send(fds[i].fd, msg_code, ans, buf_size) == EXIT_FAILURE)
                    {
                        perror("msg_send at mon_commands.c : send_ans");
                        return EXIT_FAILURE;
                    }

                    // update transmission index counter
                    transmission_index++;
                }
                else if (transmission_index == 1)
                {
                    if (msg_send(fds[i].fd, MSG_STOP_TRNS, STOP, buf_size) == EXIT_FAILURE)
                    {
                        perror("msg_send at mon_commands.c : send_ans");
                        return EXIT_FAILURE;
                    }

                    // end transmission and exit the function
                    transmission_flag = true;
                }
            }
            else continue;
        }
    }

    // return succesfully
    return EXIT_SUCCESS;
}