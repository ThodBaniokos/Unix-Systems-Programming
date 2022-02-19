// file: monitorServer.c
// implementation of the server side of the application developed on project 3
// libraries below
#include <poll.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

// custom headers below
#include "answerIntr.h"
#include "generalIntr.h"
#include "networkIntr.h"
#include "ipc_protocolIntr.h"
#include "mon_server_commandsIntr.h"
#include "monitorServerHelpersIntr.h"

// log files directory path
#define LOGSPATH "./logs/"

// global data structs used
mon_con container;
cb circural_buffer;

// void function declrations related to threads
void *read_file(void *ptr);
void init_structs(cb *circ_buf, char **dir_paths, int amount_of_dirs, pthread_t threads[], int numThreads);

// mutex used in read file
pthread_mutex_t init_mtx = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char *argv[])
{
    set_max_int_digits();
    set_max_header_digits();

    int port, numThreads, s_buf_size, c_buf_size, blm_size, in_dirs_size;
    char **input_directories_paths;

    // reading command line arguments
    if (argc_argv_manipulator_mon(argc, argv, &port, &numThreads, &s_buf_size, &c_buf_size, &blm_size, &in_dirs_size, &input_directories_paths))
    {
        perror("Cannot read command line agruments");
        return EXIT_FAILURE;
    }

    // init the container
    container = malloc(sizeof(*container));
    initContainer(&container, blm_size);

    // read the input directories to find their files
    int amount_of_input_files;
    char **input_files = getFilesPaths(input_directories_paths, in_dirs_size, &amount_of_input_files);

    // creating circural buffer of the monitor server process
    circural_buffer = cb_init(c_buf_size, in_dirs_size, input_directories_paths);
    init_cond_mtx();

    // creating threads for file reading
    pthread_t threads[numThreads];
    for(int i = 0; i < numThreads; i++)
    {
        pthread_create(&threads[i], null, read_file, circural_buffer);
    }

    init_structs(&circural_buffer, input_files, amount_of_input_files, threads, numThreads);

    // get the ip address of the machine
    char *ip = get_local_ip();

    struct sockaddr_in addport;

    // creating the socket used to read/write over
    int socket_id = create_server_socket(ip, port, &addport);

    struct sockaddr_in client;
    struct pollfd sockfd;

    // accepting the travelMonitorClient and then sending the bloom filters and the readiness message
    while (true)
    {
        int clientlen = sizeof(client);
        int new_socket_id = accept(socket_id, (struct sockaddr *) &client, (socklen_t *) &clientlen);

        if (new_socket_id == -1)
        {
            perror("accept at monitorServer.c : 82");
            exit(EXIT_FAILURE);
        }
        else
        {
            sockfd.fd = new_socket_id;
            sockfd.events = POLLIN;
        }

        sendBlooms(&container, &sockfd, s_buf_size, blm_size);
        sendReadinessMessage(&sockfd, s_buf_size);
        break;
    }

    char *buffer;

    // request received in the monitor process
    req_stats stats;

    // initialize the statistics
    stats.accepted = stats.rejected = stats.total = 0;

    bool isExiting = false;
    container->isSet = true;

    // ready to receive queries
    while (true)
    {
        bool transmission_flag = false;
        int msg_code = -1;
        int tempIndex = 0;
        int newFilesPathSize;
        int allFilesPathsSize;
        char *ans = null;
        char **newFilesPath;
        char **allFilesPaths;

        // starting loop to receive a message from the main process
        while(!transmission_flag)
        {

            // poll syscall to get ready file descriptors
            int ready_fds = poll(&sockfd, 1, 0);

            // error checking
            if (ready_fds == -1)
            {
                perror("poll at monitor.c");
                exit(EXIT_FAILURE);
            }
            else if (ready_fds == 0) continue;

            // check if the returned event of the file descriptor is POLLIN, i.e. there is data to read
            if ((sockfd.revents & POLLIN) == POLLIN)
            {
                // receive the message
                if ((buffer = msg_read(sockfd.fd, s_buf_size)) == NULL)
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

                    case MSG_CMD_ADD_VAC_REC:

                        // read all the files in the input directories
                        allFilesPaths = getFilesPaths(input_directories_paths, in_dirs_size, &allFilesPathsSize);

                        // the amount of new files is the new size minus the old one
                        newFilesPathSize = allFilesPathsSize - amount_of_input_files;

                        // allocate memory only for the new files
                        newFilesPath = calloc(newFilesPathSize, sizeof(char *));

                        // check for correct memory allocation
                        assert(newFilesPath != null);

                        // copy only the new files found
                        for(int i = 0; i < allFilesPathsSize; i++)
                        {
                            if(!isFilePresent(input_files, amount_of_input_files, allFilesPaths[i]))
                            {
                                // if the file is new then allocate memory for it's path
                                newFilesPath[tempIndex] = calloc(strlen(allFilesPaths[i]) + 1, sizeof(char));

                                // check for correct memory allocation
                                assert(newFilesPath[tempIndex] != null);

                                // copy the new file path
                                strcpy(newFilesPath[tempIndex], allFilesPaths[i]);

                                // update the temporary index variable
                                tempIndex++;
                                if(tempIndex == newFilesPathSize) break;
                            }
                        }

                        // start the child threads for the data structs information update
                        init_structs(&circural_buffer, newFilesPath, newFilesPathSize, threads, numThreads);

                        // delete previously allocated memory of the new files and all files
                        for(int i = 0; i < newFilesPathSize; i++)
                        {
                            if(newFilesPath[i]) free(newFilesPath[i]);
                        }

                        if (newFilesPath) free(newFilesPath);

                        for(int i = 0; i < amount_of_input_files; i++)
                        {
                            if(input_files[i]) free(input_files[i]);
                        }

                        if (input_files) free(input_files);

                        // set the pointers to point to the newly read file paths
                        input_files = allFilesPaths;
                        amount_of_input_files = allFilesPathsSize;

                        // send the bloom filters and readiness message to the travelMonitorClient
                        sendBlooms(&container, &sockfd, s_buf_size, blm_size);

                        sendReadinessMessage(&sockfd, s_buf_size);
                        break;

                    case MSG_CMD_EXIT:

                        // set is exiting boolean to true in order to stop the monitorServer
                        isExiting = true;
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

        // if msg_code is not set then just dont send anythin
        if (msg_code != -1) send_ans(&sockfd, ans, s_buf_size, msg_code);

        // stop server execution
        if (isExiting) break;
    }

    // cleanup memory
    serverCleanup(socket_id, container, &stats, LOGSPATH, threads, numThreads, &circural_buffer, input_files, amount_of_input_files, input_directories_paths, in_dirs_size);

    return EXIT_SUCCESS;
}

// function used by main thread to add paths to the circural buffer
void init_structs(cb *circ_buf, char **dir_paths, int amount_of_dirs, pthread_t threads[], int numThreads)
{
    // used as index to determine which path is going to be inserted
    paths_left = 0;

    // how many paths we're inserting now
    input_dir_paths_size = amount_of_dirs;

    // as long as there are paths to insert keep adding
    while (input_dir_paths_size > 0)
    {
        cb_add(*circ_buf, dir_paths[paths_left]);
        input_dir_paths_size--;
        paths_left++;
        pthread_cond_signal(&buffer_not_empty);
    }

    // wait for the threads to finish initialization
    // since we've reached this point there are no files
    // to insert only to read and initiliaze
    // we do this in order to send the correct bloom filters
    // to the travelMonitorClient
    for(;;)
    {
        // sleep for 0.1 secs, to give the program time to update the item counter of the buffer
        usleep(100000);

        // if the buffer is empty break
        if ((*circ_buf)->item_count <= 0) break;
    }

    return;
}

// function passed to the child threads
void *read_file(void *ptr)
{
    cb circ_buf = (cb)ptr;
    char *path = null;

    // repeat until the threads receive the text END
    while((strcmp((path = cb_get(circ_buf)), "END")) != 0)
    {
        // notify main thread
        pthread_cond_signal(&buffer_not_full);

        // lock mutex
        pthread_mutex_lock(&init_mtx);

        // open the file
        FILE *stream = fopen(path, "r");

        // lock the file in order to avoid data races from other threads, extra protection
        flockfile(stream);

        // initialize the data structs of the container
        initDataStructs(&container, &stream);

        // unlock file
        funlockfile(stream);

        // close file
        fclose(stream);

        // unlock mutex
        pthread_mutex_unlock(&init_mtx);
    }

    return null;
}
