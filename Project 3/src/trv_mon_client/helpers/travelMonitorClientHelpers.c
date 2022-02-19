// file: travelMonitorHelpers.c
// helper functions used in travelMonitor
// libs below
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <dirent.h>
#include <sys/stat.h>

// custom headers below
#include "blmIntr.h"
#include "utilsIntr.h"
#include "commonIntr.h"
#include "networkIntr.h"
#include "hashTableIntr.h"
#include "convertersIntr.h"
#include "linkedListIntr.h"
#include "ipc_protocolIntr.h"
#include "errorCheckingIntr.h"
#include "travelMonitorClientHelpersIntr.h"

#define FIXED_ARGS_SIZE 13

// reads command line arguments if given
int argc_argv_manipulator(int argc, char **argv, int *num_monitors, int *numThreads, int  *socket_buf_size, int *cyclic_buf_size, int *bloom_size, char **input_dir_path)
{
    // check if the arguments given are more than the programs name
    if(argc == 13)
    {
        // start reading from 1 because the first argv argument is the program name
        char *arg;
        bool correctArgs = false;
        struct stat dir;

        for(int i = 1; i < argc; i++)
        {
            // getting the flag
            arg = argv[i];

            // checking the flags for input and config files
            // if there are these flags open the files given by the paths
            if(!strcmp(arg, "-m"))
            {
                // number of monitors
                if (!isNumber(argv[i + 1], strlen(argv[i + 1]))) return EXIT_FAILURE;
                correctArgs = true;
                *num_monitors = atoi(argv[i + 1]);
            }
            else if(!strcmp(arg, "-b"))
            {
                // socket buffer size
                if (!isNumber(argv[i + 1], strlen(argv[i + 1]))) return EXIT_FAILURE;
                correctArgs = true;
                *socket_buf_size = (size_t)atoi(argv[i + 1]);
            }
            else if(!strcmp(arg, "-s"))
            {
                // bloom filter size
                if (!isNumber(argv[i + 1], strlen(argv[i + 1]))) return EXIT_FAILURE;
                correctArgs = true;
                *bloom_size = atoi(argv[i + 1]);
            }
            else if(!strcmp(arg, "-i"))
            {
                // input directory path
                if (stat(argv[i + 1], &dir) && S_ISDIR(dir.st_mode)) return EXIT_FAILURE;
                correctArgs = true;
                *input_dir_path = argv[i + 1];
            }
            else if(!strcmp(arg, "-c"))
            {
                // cyclic buffer size
                if (!isNumber(argv[i + 1], strlen(argv[i + 1]))) return EXIT_FAILURE;
                correctArgs = true;
                *cyclic_buf_size = atoi(argv[i + 1]);
            }
            else if(!strcmp(arg, "-t"))
            {
                // number of threads
                if (!isNumber(argv[i + 1], strlen(argv[i + 1]))) return EXIT_FAILURE;
                correctArgs = true;
                *numThreads = atoi(argv[i + 1]);
            }
        }

        if (!correctArgs) return EXIT_FAILURE;
    }
    else return EXIT_FAILURE;

    // arguments read successfuly
    return EXIT_SUCCESS;
}

// read the sub directories in the input directory given from the command line arguments
int readDirectories(const char *input_dir_path, char ***full_paths, int *inputDirs)
{
    struct dirent **in_dir;
    int amountOfSubDirs = scandir(input_dir_path, &in_dir, 0, alphasort);

    if (amountOfSubDirs < 0)
    {
        perror("Cannot scan directory");
        return EXIT_FAILURE;
    }
    else
    {
        // allocate memory of a strings array to copy the full paths of the directories
        // - 2 because we're not counting . and .. directories returned from scandir()
        (*full_paths) = calloc((amountOfSubDirs - 2), sizeof(char *));

        // check for correct memory allocation
        assert(full_paths != null);

        // full paths array index
        int index = 0;

        // copy the paths of the directories
        for(int i = 2; i < amountOfSubDirs; i++) // skiping . and .. sub dirs
        {
            // allocate memory for the path
            (*full_paths)[index] = malloc((strlen(input_dir_path) + strlen(in_dir[i]->d_name) + 2) * sizeof(char)); // +2 for '/' and '\0'

            // check for correct memory allocation
            assert((*full_paths)[index] != null);

            // copy the path with the '/' characters when it's needed
            strcpy((*full_paths)[index], input_dir_path);
            strcat((*full_paths)[index], "/");
            strcat((*full_paths)[index], in_dir[i]->d_name);

            // update index
            index++;
        }
    }

    // free memory of the dirent struct if exists
    for(int i = 0; i < amountOfSubDirs; i++)
    {
        if (in_dir[i]) free(in_dir[i]);
    }

    if (in_dir) free(in_dir);

    // set the amount of subdirectories
    *inputDirs = amountOfSubDirs - 2;

    return EXIT_SUCCESS;
}

// compares two dates as string
int compareDates(Item date1, Item date2)
{
    return strcmp(((req_info)date1)->date, (char *)date2);
}

int compareHtCon(Item con, Item country)
{
    return strcmp(((ht_con)con)->country, (char *)country);
}

void printHtCon(Item to_print)
{
    printf("%s\n", ((ht_con)to_print)->country);
    printList(((ht_con)to_print)->viruses);
    return;
}

void destroyHtCon(Item discard)
{
    if (((ht_con)discard)->country) free(((ht_con)discard)->country);
    destroyList(((ht_con)discard)->viruses);
    free(discard);
    return;
}

void destroyDates(Item item)
{
    if (((req_info)item)->date) free(((req_info)item)->date);
    free(item);
    return;
}

void printDates(Item item)
{
    printf("Date : %s, accepted : %d, rejected : %d\n", ((req_info)item)->date, ((req_info)item)->accepted, ((req_info)item)->rejected);
    return;
}

// void functions below
void general_initialization(void)
{
    // init signal handlers and set the max
    // length for int conversion and header of message
    set_max_int_digits();
    set_max_header_digits();
}

// initializes each monitor process and other needed fields such as the fields in the pollfd struct
void process_initialization(msi *proc_info, int numMonitors, int numThreads, char *input_dir_path, char **full_paths, int subdirs, int s_buf_size, int c_buf_size, int init_port, int blm_size, struct pollfd *all_file_descs)
{
    // set input subdirs for each process
    distributeSubDirs(proc_info, input_dir_path, full_paths, numMonitors, subdirs);

    // create the monitor processes and initialize needed fields
    setupMonitors(proc_info, numMonitors, numThreads, init_port, s_buf_size, c_buf_size, blm_size);

    // create the needed variables for the communication over sockets
    create_client_sockets(proc_info, numMonitors, get_local_ip());

    // init above array
    initPollfdStruct(proc_info, numMonitors, all_file_descs, numMonitors);

    // start connecting to each server
    connect_to_servers(proc_info, numMonitors);

    // receiving the bloom filters
    receiveBlooms(proc_info, numMonitors, all_file_descs, numMonitors, s_buf_size);

    // ready to send queries
    readinessCheck(proc_info, numMonitors, s_buf_size, all_file_descs, numMonitors);

    return;
}

// deletes all allocated memory of information about the monitor process
// the named pipes and closes the files
void deleteMonitorProcessInfo(msi discard[], int procs)
{
    // for each monitor process delete named pipes, memory allocated for
    // hash tables, bloom filters if exists
    for(int i = 0; i < procs; i++)
    {
        if (!discard[i].subdirsAmount) continue;

        for(int j = 0; j < discard[i].subdirsAmount; j++)
        {
            free(discard[i].subdirs[j]);
            free(discard[i].countries[j]);
        }
        free(discard[i].subdirs);
        free(discard[i].countries);

        for (int j = 0; j < discard[i].blooms_amount; j++)
        {
            blmDestroy(discard[i].blooms[j]);
        }

        free(discard[i].blooms);
        destroyHash(discard[i].travel_req_queries);
    }

    return;
}

// distribute the sub directories with round robin
void distributeSubDirs(msi *proc_info, const char *input_dir_path, char **full_paths, int processes, int subdirs)
{
    struct dirent **in_dir;

    // reading the input directory with alphabetical order and getting it's size
    int amountOfSubDirs = scandir(input_dir_path, &in_dir, 0, alphasort);

    // calculating the amount of subdirs of each monitor process
    int subDirsPerProcess = subdirs / processes;

    // if there's a remainder calculate it with a mod operation
    int remainder = subdirs % processes;

    // more processes than sub directories, every process is getting one sub dir
    // and we're not creating more than subdir processes
    if (remainder == subdirs)
    {
        // each child process monitors a single input directory
        for(int i = 0; i < subdirs; i++)
        {
            // setting the subdirs amount of every monitor
            proc_info[i].subdirsAmount = 1;

            // allocating enough memory for the subdirs and auxiliary countries arrays
            proc_info[i].subdirs = calloc(proc_info[i].subdirsAmount, sizeof(char *));

            // checking for correct memory allocation
            assert(proc_info[i].subdirs != null);

            proc_info[i].countries = calloc(proc_info[i].subdirsAmount, sizeof(char *));

            // checking for correct memory allocation
            assert(proc_info[i].countries != null);

            // copying full path of the input directory in the subdirs array of the i'th process
            proc_info[i].subdirs[0] = full_paths[i];

            // allocating memory for the country i'th country
            proc_info[i].countries[0] = calloc((strlen(in_dir[i + 2]->d_name) + 1), sizeof(char));

            // checking for correct memory allocation
            assert(proc_info[i].countries[0] != null);

            // copying the contents of the dirent struct in the countries array
            strcpy(proc_info[i].countries[0], in_dir[i + 2]->d_name);
        }

        // setting the subdirs of every other cell of the mpi struct to 0
        // used for error checking, if needed
        for(int i = subdirs; i < processes; i++) proc_info[i].subdirsAmount = 0;
    }
    else
    {
        // less processes than subdirs amount
        // initialize the variables in the struct with the appropriate values
        // each monitor process has subDirsPerProcess amount of subdirectories
        for(int i = 0; i < processes; i++) proc_info[i].subdirsAmount = subDirsPerProcess;

        // some might have more, start from the first until the remainder of the modulo operation
        // at line 338 and updating the subdirs amount counter by one
        for(int i = 0; i < remainder; i++) proc_info[i].subdirsAmount++;

        // allocating memory for the strings arrays
        for(int i = 0; i < processes; i++)
        {
            // allocating memory for the paths array
            proc_info[i].subdirs = calloc(proc_info[i].subdirsAmount, sizeof(char *));

            // checking for correct memory allocation
            assert(proc_info[i].subdirs != null);

            // allocating memory for the countries names array
            proc_info[i].countries = malloc(proc_info[i].subdirsAmount * sizeof(char *));

            // checking for correct memory allocation
            assert(proc_info[i].countries != null);
        }

        // copy the full paths in the array of the paths and the countries names to the countries array
        // index and loops variables are auxiliary and used to determine in which process holder each information will be copied
        // loops is used to determine the process holder, index is used to determine the process holder's secondary array
        int index = 0, loops = 0;
        for(int i = 0; i < subdirs; i++)
        {
            // copy the full path of the subdirectory in the process holder
            proc_info[loops].subdirs[index] = full_paths[i];

            // allocate memory for the country
            proc_info[loops].countries[index] = calloc((strlen(in_dir[i + 2]->d_name) + 1), sizeof(char));

            // check for correct memory allocation
            assert(proc_info[loops].countries[index] != null);

            // copy the country
            strcpy(proc_info[loops].countries[index], in_dir[i + 2]->d_name);

            // update loop counter
            loops++;

            // if the loop counter is the same as the processes
            // reset it and update the index counter
            if (loops == processes)
            {
                loops = 0;
                index++;
            }
        }

        // free allocated memory for the dirent struct if exists
        for(int i = 0; i < amountOfSubDirs; i++)
        {
            if (in_dir[i]) free(in_dir[i]);
        }

        free(in_dir);
    }

    return;
}

// set up the process holder and initialize the fields, fork() for the processes
void setupMonitors(msi *proc_info, int numMonitors, int numThreads, int init_port, int s_buf_size, int c_buf_size, int blm_size)
{
    // creating the processes and setting every value needed in the process info array
    pid_t pid;

    // for all the monitors we need to create
    for(int i = 0; i < numMonitors; i++)
    {
        // set the port number
        proc_info[i].port_num = init_port + i;

        int size;

        // build the arguments array needed by execv
        char **args = buildArgs("./monitorServer", proc_info[i], numThreads, s_buf_size, c_buf_size, blm_size, &size);

        // create a new process
        pid = fork();

        // if pid is less than zero then an error occured
        if (pid < 0)
        {
            perror("Fork failed");

            // deallocate memory if exists
            deleteMonitorProcessInfo(proc_info, numMonitors);

            // send failure to os
            exit(EXIT_FAILURE);
        }
        else if (pid == 0)
        {
            // inside child process
            // execute the monitor process executable
            execvp("./monitorServer", args);
        }
        else
        {
            // inside parent process
            // delete allocated memory for the arguments array if exists
            for(int j = 0; j < size; j++)
            {
                if (args[j]) free(args[j]);
            }

            if (args) free(args);

            // inside parent process
            // set monitor process id
            proc_info[i].mon_process_id = pid;

            // initialization is not yet finished, set isSet flag to false
            proc_info[i].isSet = false;
            proc_info[i].isReady = false;

            // allocate memory of the hash table for the queries sent to this process
            proc_info[i].travel_req_queries = createHash(hashFunction, printHtCon, destroyHtCon, compareHtCon);
        }
    }
}

// initialize the pollfd struct inside the process holders struct mpi
void initPollfdStruct(msi *proc_info, int numMonitors, struct pollfd *all_file_descs, int all_file_descs_size)
{
    // init the all file descriptors inside the poll fd struct
    // iterate through all the cells
    for(int i = 0; i < all_file_descs_size; i++)
    {
        // initialize the read file descs
        all_file_descs[i].fd = proc_info[i].socketfd.fd;
        all_file_descs[i].events = proc_info[i].socketfd.events;
    }

    return;
}

// receive the initialized bloom filters from every process
void receiveBlooms(msi *proc_info, int numMonitors, struct pollfd *all_file_descs, int all_file_descs_size, int buf_size)
{
    // transmission index determines if the main process received every bloom filter from all the child processes
    int transmision_index = 0;
    bool transmission_flag = false;

    // buffer used to store the message
    char *buffer;

    // counter array for every monitor process to "know" how many bloom filter we've read so far
    int bloom_index[numMonitors];
    for (int i = 0; i < numMonitors; i++)
    {
        bloom_index[i] = 0;

        // checking if a monitor processes is already set, meaning one or more child processes died
        // and we reinitialize them
        if (proc_info[i].isSet) transmision_index++;
    }

    // loop until message with code stop sent
    while (!transmission_flag)
    {
        // poll() syscall to find ready file descriptors
        int ready_fds = poll(all_file_descs, all_file_descs_size, 0);

        // error checking
        if (ready_fds == -1)
        {
            perror("poll at monitor.c");
            exit(EXIT_FAILURE);
        }
        else if (ready_fds == 0) continue;

        for(int i = 0; i < all_file_descs_size; i++)
        {
            // check if the process indicated from i is already set or not
            // and if it is continue
            if (proc_info[i].isSet == true) continue;

            // check if there's data to read
            if ((all_file_descs[i].revents & POLLIN) == POLLIN)
            {
                // reading data and storing it to buffer
                if ((buffer = msg_read(all_file_descs[i].fd, buf_size)) == NULL)
                {
                    perror("msg_read at travelMonitor.c");
                    exit(EXIT_FAILURE);
                }

                // used to get the string representation of the bloom filter
                char *bloom_str;

                // switch case to decode the received message
                switch (get_msg_code(buffer))
                {
                    // received the amount of bloom filters from the process
                    case MSG_AMNT_BLM:

                        // allocate memory for all the bloom filters
                        proc_info[i].blooms = malloc(atoi(buffer + max_header_digits) * sizeof(blm));

                        // check for correct memory allocation
                        assert(proc_info[i].blooms != null);

                        // set the amount of bloom filters
                        proc_info[i].blooms_amount = atoi(buffer + max_header_digits);
                        break;

                    // received a bloom filter
                    case MSG_BLM:

                        // get the string representation of the bloom filter
                        bloom_str = buffer + max_header_digits;

                        // decode the string to a bloom filter with the same fields
                        blm temp;
                        temp = string_to_blm(bloom_str);

                        // set the bloom filter in the blm's array and update the bloom_index array
                        proc_info[i].blooms[bloom_index[i]++] = temp;
                        break;

                    // received message stop transmission
                    case MSG_STOP_TRNS:

                        // set the current process isSet flag to true
                        proc_info[i].isSet = true;
                        transmision_index++;
                        break;
                }

                // free allocated memory if exists
                if (buffer) free(buffer);

                // check if we've received everything from every process
                if (transmision_index == numMonitors)
                {
                    transmission_flag = true;
                    break;
                }
            }
        }
    }

    return;
}

void readinessCheck(msi *proc_info, int numMonitors, int buf_size, struct pollfd *all_file_descs, int all_file_descs_size)
{
    // transmission index determines if the main process received every bloom filter from all the child processes
    int transmision_index = 0;
    bool transmission_flag = false;

    // buffer used to store the message
    char *buffer;

    for(int i = 0; i < numMonitors; i++)
    {
        if (proc_info[i].isReady == true) transmision_index++;
    }

    // loop until message with code stop sent
    while (!transmission_flag)
    {
        // poll() syscall to find ready file descriptors
        int ready_fds = poll(all_file_descs, numMonitors, 0);

        // error checking
        if (ready_fds == -1)
        {
            perror("poll at monitor.c");
            exit(EXIT_FAILURE);
        }
        else if (ready_fds == 0) continue;

        for(int i = 0; i < all_file_descs_size; i++)
        {
            // check if there's data to read
            if ((all_file_descs[i].revents & POLLIN) == POLLIN)
            {
                // reading data and storing it to buffer
                if ((buffer = msg_read(all_file_descs[i].fd, buf_size)) == NULL)
                {
                    perror("msg_read at travelMonitor.c");
                    exit(EXIT_FAILURE);
                }

                // switch case to decode the received message
                switch (get_msg_code(buffer))
                {
                    // received the amount of bloom filters from the process
                    case MSG_Q_READINESS:

                        proc_info[i].isReady = true;
                        break;

                    // received message stop transmission
                    case MSG_STOP_TRNS:
                        transmision_index++;
                        break;
                }

                // free allocated memory if exists
                if (buffer) free(buffer);

                // check if we've received everything from every process
                if (transmision_index == numMonitors)
                {
                    transmission_flag = true;
                    break;
                }
            }
        }
    }

    return;
}

// char functions below
char **buildArgs(char *programName, msi proc_info, int numThreads, int s_buf_size, int c_buf_size, int blm_size, int *size)
{
    // set the size of the array
    *size = FIXED_ARGS_SIZE + proc_info.subdirsAmount;

    // allocate memory for the args array
    char **args_array = calloc(*size, sizeof(char *));

    // check for correct memory allocation
    assert(args_array != null);

    char *temp;

    // copy every needed argument for the monitor server program
    // set program name
    args_array[0] = calloc(strlen(programName) + 1, sizeof(char));
    strcpy(args_array[0], programName);

    // set port number
    args_array[1] = calloc(strlen("-p") + 1, sizeof(char));
    strcpy(args_array[1], "-p");
    temp = int_to_string(proc_info.port_num);
    args_array[2] = calloc(strlen(temp) + 1, sizeof(char));
    assert(args_array[2] != null);
    strcpy(args_array[2], temp);
    free(temp);

    // set the number of threads
    args_array[3] = calloc(strlen("-t") + 1, sizeof(char));
    strcpy(args_array[3], "-t");
    temp = int_to_string(numThreads);
    args_array[4] = calloc(strlen(temp) + 1, sizeof(char));
    assert(args_array[4] != null);
    strcpy(args_array[4], temp);
    free(temp);

    // set the socket buffer size
    args_array[5] = calloc(strlen("-b") + 1, sizeof(char));
    strcpy(args_array[5], "-b");
    temp = int_to_string(s_buf_size);
    args_array[6] = calloc(strlen(temp) + 1, sizeof(char));
    assert(args_array[6] != null);
    strcpy(args_array[6], temp);
    free(temp);

    // set the cyclic buffer size
    args_array[7] = calloc(strlen("-c") + 1, sizeof(char));
    strcpy(args_array[7], "-c");
    temp = int_to_string(c_buf_size);
    args_array[8] = calloc(strlen(temp) + 1, sizeof(char));
    assert(args_array[8] != null);
    strcpy(args_array[8], temp);
    free(temp);

    // set the bloom filter size
    args_array[9] = calloc(strlen("-s") + 1, sizeof(char));
    strcpy(args_array[9], "-s");
    temp = int_to_string(blm_size);
    args_array[10] = calloc(strlen(temp) + 1, sizeof(char));
    assert(args_array[10] != null);
    strcpy(args_array[10], temp);
    free(temp);

    // copy the paths of the sub directories
    for(int i = 0; i < proc_info.subdirsAmount; i++)
    {
        // if its null continue
        if (!proc_info.subdirs[i]) continue;
        args_array[i + 11] = calloc(strlen(proc_info.subdirs[i]) + 1, sizeof(char));
        assert(args_array[i + 11] != null);
        strcpy(args_array[i + 11], proc_info.subdirs[i]);
    }

    return args_array;
}
