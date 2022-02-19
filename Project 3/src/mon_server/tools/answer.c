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

    if (msg_code != MSG_STOP_TRNS)
    {
        if (msg_send(fds->fd, msg_code, ans, buf_size) == EXIT_FAILURE)
        {
            perror("msg_send at mon_commands.c : send_ans");
            return EXIT_FAILURE;
        }
    }

    if (msg_send(fds->fd, MSG_STOP_TRNS, STOP, buf_size) == EXIT_FAILURE)
    {
        perror("msg_send at mon_commands.c : send_ans");
        return EXIT_FAILURE;
    }

    if (ans) free(ans);

    // return succesfully
    return EXIT_SUCCESS;
}