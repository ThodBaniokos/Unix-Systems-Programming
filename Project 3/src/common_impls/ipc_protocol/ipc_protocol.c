// file : ipc_protocol.c
// implementation of the custom ipc protocol
// libs below
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <assert.h>

// custom headers below
#include "ipc_protocolIntr.h"

// globals
int max_int_digits = 0;
int max_header_digits = 0;

// Explanation of the message format for the custom ipc protocol
// The composition of a message is as follows:
// a char array of custom length each time a message is sent
// at the first cell is the message code, since we dont have
// message codes greater than 127 we can assign the message code
// as follows : message_array[0] = message_code
// after that is the header of the message which represent the length
// of the actual message, the length of the header is the length needed
// store an int up to INT_MAX (0 - 2147483647) meaning how many characters
// are in the actual message and then the actual message.
//                    msg_code + header + actual message
// size in bytes is :    1   + at most 10 + the number that the header has stored

// function implementations below
// int functions
// message code is stored in the first cell of the message buffer
int get_msg_code(char *msg) { return msg[0]; }

// sending message
int msg_send(int file_desc, int msg_code, char *msg, int buf_size)
{
    // get the length of the message to send
    size_t msg_length = strlen(msg);

    // create a buffer for the complete message,
    // composed of the message code, the header and the actual message
    char comp_msg[max_header_digits + msg_length + 1];

    // initialize complete message buffer to null
    memset(comp_msg, '\0', max_header_digits + msg_length + 1);

    // set message code
    comp_msg[0] = msg_code;

    // setting the header of the message, i.e. the actual size of the message to send
    // set after the first cell of the complete message in order to not overwrite
    // the message code
    snprintf(comp_msg + 1, max_int_digits + 1, "%0*ld", max_int_digits, msg_length);

    // setting the actual message to the buffer
    snprintf(comp_msg + max_header_digits, msg_length + 1, "%s", msg);

    // setting the new full message length
    msg_length = strlen(comp_msg);

    // variables used to calculate chars to write
    int sent = 0, difference, to_write, written;

    // actual buffer used to write
    char buff[buf_size];

    // initialize buffer to null
    memset(buff, '\0', buf_size);

    while (sent < msg_length)
    {
        // how many chars are left to write
        difference = msg_length - sent;

        // set the new write amount
        to_write = (difference < buf_size) ? difference : buf_size;

        // copy the specific bytes to write from the complete message to the buffer
        // copy after the amount of already sent chars
        //strncpy(buff, comp_msg + sent, to_write);
        memcpy(buff, comp_msg + sent, to_write);

        // write to the buffer
        if ((written = write(file_desc, buff, to_write)) < 0)
        {
            perror("Write at ipc_protocol.c : 89");
            return EXIT_FAILURE;
        }

        // update the sent bytes counter
        sent += written;
    }

    return EXIT_SUCCESS;
}

// char functions
// read the message that was sent
bool read_specified_bytes(int file_desc, int bytes_to_read, char *msg, int buffer_size, char *buffer)
{
    int received = 0;
    int difference;

    // read until we've received all the bytes
    while (received < bytes_to_read)
    {
        // how many bytes to read left
        difference = bytes_to_read - received;

        // how many bytes we're going to read
        int to_read = (difference < buffer_size) ? difference : buffer_size;

        // read *to_read* amount of bytes
        int just_read = read(file_desc, buffer, to_read);

        // check for errors
        if (just_read < 0)
        {
            // small buffer error (EAGAIN) of syscall interupt (EINTR), ignore them and continue
            if (errno == EAGAIN || errno == EINTR) continue;

            // if not something else happened, terminate
            perror("Read at ipc_protocol.c : 126");
            return false;
        }
        else if (just_read == 0)
           continue;

        // copy the read bytes in the buffer
        memcpy(msg + received, buffer, to_read);

        // update received bytes counter
        received += just_read;
    }

    return true;
}

char *msg_read(int file_desc, int buf_size)
{
    // create a buffer for the message that is about to be read
    char buff[buf_size];

    // initialize buffer to null
    memset(buff, '\0', buf_size);

    // first read the message code
    char msg_code_header[max_header_digits + 1];

    // initialize buffer to null
    memset(msg_code_header, '\0', max_header_digits + 1);

    // read the message code
    if (read_specified_bytes(file_desc, 1, msg_code_header, buf_size, buff) == false)
    {
        perror("read_specified_bytes at ipc_protocol.c : 159");
        return NULL;
    }

    memset(buff, '\0', buf_size);

    // read the header of the message
    if (read_specified_bytes(file_desc, max_int_digits, msg_code_header + 1, buf_size, buff) == false)
    {
        perror("read_specified_bytes at ipc_protocol.c : 168");
        return NULL;
    }

    memset(buff, '\0', buf_size);

    // set null terminator
    msg_code_header[max_header_digits] = '\0';

    // get the actual message size to read
    int msg_size = atoi(msg_code_header + 1);

    // read the actual message
    // create a buffer for the complete message
    char *full_msg = calloc((max_header_digits + msg_size + 1), sizeof(char));

    // check for correct memory allocation
    assert(full_msg != NULL);

    // copy message code and header to the full message buffer
    memcpy(full_msg, msg_code_header, max_header_digits);

    if (read_specified_bytes(file_desc, msg_size, full_msg + max_header_digits, buf_size, buff) == false)
    {
        perror("read_specified_bytes at ipc_protocol.c : 192");
        return NULL;
    }

    // set null terminator to the full message buffer
    full_msg[max_header_digits + msg_size] = '\0';

    // return the read message
    return full_msg;
}

// get the header of the message in string format
char *get_msg_header(char *msg)
{
    // return the actual header
    return msg + max_header_digits;
}

// void functions
// sets the max length of chars needed to store ints till INT_MAX
void set_max_int_digits(void)
{
    max_int_digits = snprintf(NULL, max_int_digits, "%d", INT_MAX);
    return;
}

// sets the max header length which is the above length plus one
void set_max_header_digits(void)
{
    max_header_digits = max_int_digits + 1;
    return;
}