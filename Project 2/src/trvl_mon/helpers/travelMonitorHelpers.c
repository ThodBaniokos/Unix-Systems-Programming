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
#include "hashTableIntr.h"
#include "convertersIntr.h"
#include "linkedListIntr.h"
#include "sig_actionsIntr.h"
#include "ipc_protocolIntr.h"
#include "errorCheckingIntr.h"
#include "travelMonitorHelpersIntr.h"

// reads command line arguments if given
int argc_argv_manipulator(int argc, char **argv, int *num_monitors, int *buf_size, int *bloom_size, char **input_dir_path)
{
    // check if the arguments given are more than the programs name
    if(argc == 9)
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
                if (!isNumber(argv[i + 1], strlen(argv[i + 1]))) return EXIT_FAILURE;
                correctArgs = true;
                *num_monitors = atoi(argv[i + 1]);
            }
            else if(!strcmp(arg, "-b"))
            {
                if (!isNumber(argv[i + 1], strlen(argv[i + 1]))) return EXIT_FAILURE;
                correctArgs = true;
                *buf_size = (size_t)atoi(argv[i + 1]);
            }
            else if(!strcmp(arg, "-s"))
            {
                if (!isNumber(argv[i + 1], strlen(argv[i + 1]))) return EXIT_FAILURE;
                correctArgs = true;
                *bloom_size = atoi(argv[i + 1]);
            }
            else if(!strcmp(arg, "-i"))
            {
                if (stat(argv[i + 1], &dir) && S_ISDIR(dir.st_mode)) return EXIT_FAILURE;
                correctArgs = true;
                *input_dir_path = argv[i + 1];
            }
        }

        if (!correctArgs) return EXIT_FAILURE;
    }
    else return EXIT_FAILURE;

    // arguments read successfuly
    return EXIT_SUCCESS;
}

// creates named pipes for inter process communication
int create_named_pipes(const char *read_format, const char *write_format, int proc_amount, int subdirs)
{
    int n_pipe_num;
    // char *read_name;
    // char *write_name;
    char num[max_int_digits];
    int fifosAmount = (proc_amount < subdirs) ? proc_amount : subdirs;

    // create the named pipes, two for each process, one for reading and one for writing
    // format : main_proc_write_X, child_proc_write_X
    for(int i = 0; i < 2*fifosAmount; i+=2)
    {
        // create the number of the named pipe
        n_pipe_num = i / 2 + 1;
        sprintf(num, "%d", n_pipe_num);

        // create the read pipe full path
        char read_name[strlen(num) + strlen(read_format) + 1];

        // initialize the string array to null
        memset(read_name, '\0', strlen(num) + strlen(read_format) + 1);

        // copy the full path of the read fifo
        strcpy(read_name, read_format);
        strcat(read_name, num);

        // create the write pipe full path
        char write_name[strlen(num) + strlen(write_format) + 1];

        // initialize the string array to null
        memset(write_name, '\0', strlen(num) + strlen(write_format) + 1);

        // copy the full path of the write fifo
        strcpy(write_name, write_format);
        strcat(write_name, num);

        // create the fifo's in the ./tmp folder
        if (mkfifo(read_name, 0777) < 0) return EXIT_FAILURE;
        if (mkfifo(write_name, 0777) < 0) return EXIT_FAILURE;
    }

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
    signal(SIGINT, sigint_handler);
    signal(SIGQUIT, sigquit_handler);
    signal(SIGCHLD, sigchld_handler);
    set_max_int_digits();
    set_max_header_digits();
}

// initializes each monitor process and other needed fields such as the fields in the pollfd struct
void process_initialization(mpi *proc_info, int numMonitors, char *input_dir_path, char **full_paths, int subdirs, char *read_fifo_path, char *write_fifo_path, int buf_size, int blm_size, struct pollfd *all_file_descs)
{
    // set input subdirs for each process
    distributeSubDirs(proc_info, input_dir_path, full_paths, numMonitors, subdirs);

    // create the monitor processes and initialize needed fields
    setupMonitors(proc_info, numMonitors, read_fifo_path, write_fifo_path);

    // init above array
    initPollfdStruct(proc_info, numMonitors, all_file_descs, numMonitors * FILE_DESCRIPTORS);

    // first we're sending the buffer size, and the bloom filter size
    sendBufferAndBloomSize(proc_info, numMonitors, buf_size, blm_size, all_file_descs);

    // sending amount of subdirs of each monitor
    sendSubDirsAmount(proc_info, numMonitors, buf_size, all_file_descs);

    // sending the subdirs
    sendSubDirs(proc_info, numMonitors, buf_size, all_file_descs);

    // receiving the bloom filters
    receiveBlooms(proc_info, numMonitors, all_file_descs, numMonitors * FILE_DESCRIPTORS, buf_size);

    return;
}

// deletes all allocated memory of information about the monitor process
// the named pipes and closes the files
void deleteMonitorProcessInfo(mpi discard[], int procs)
{
    // for each monitor process delete named pipes, memory allocated for
    // hash tables, bloom filters if exists
    for(int i = 0; i < procs; i++)
    {
        if (!discard[i].subdirsAmount) continue;

        if (discard[i].read_np)
        {
            if (discard[i].read_fd != -1) close(discard[i].read_fd);
            unlink(discard[i].read_np);
            free(discard[i].read_np);
        }

        if (discard[i].write_np)
        {
            if (discard[i].write_fd != -1) close(discard[i].write_fd);
            unlink(discard[i].write_np);
            free(discard[i].write_np);
        }

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

// copy named pipe path to the monitor process info
void copyNamedPipesPath(const char *write_fifo_path_name, const char *read_fifo_path_name, mpi *proc_info, int index)
{

    char num[max_int_digits], *path;

    // first initialize the values of the process info array with every available
    // value, first we're creating the read and write named pipes
    // used to get the correct named pipe
    sprintf(num, "%d", index + 1);

    // build the read path
    path = malloc((strlen(num) + strlen(write_fifo_path_name) + 1) * sizeof(char));
    strcpy(path, write_fifo_path_name);
    strcat(path, num);
    proc_info[index].read_np = path;

    // build the write path
    path = malloc((strlen(num) + strlen(read_fifo_path_name) + 1) * sizeof(char));
    strcpy(path, read_fifo_path_name);
    strcat(path, num);
    proc_info[index].write_np = path;

    return;
}

// distribute the sub directories with round robin
void distributeSubDirs(mpi *proc_info, const char *input_dir_path, char **full_paths, int processes, int subdirs)
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
void setupMonitors(mpi *proc_info, int numMonitors, char *read_fifo_path_name, char *write_fifo_path_name)
{
    // creating the processes and setting every value needed in the process info array
    pid_t pid;

    for(int i = 0; i < numMonitors; i++)
    {
        // copy named pipe paths for this process
        copyNamedPipesPath(write_fifo_path_name, read_fifo_path_name, proc_info, i);

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
            execl("./Monitor", "./Monitor", proc_info[i].write_np, proc_info[i].read_np, NULL);
        }
        else
        {
            // inside parent process
            // set monitor process id
            proc_info[i].mon_process_id = pid;

            // open read and write fifos
            // read fifo
            if ((proc_info[i].read_fd = open(proc_info[i].read_np, O_RDONLY | O_NONBLOCK)) < 0)
            {
                perror("Cannot open read fifo at traverMonitor.c");
                deleteMonitorProcessInfo(proc_info, numMonitors);
                exit(EXIT_FAILURE);
            }

            // write fifo
            if ((proc_info[i].write_fd = open(proc_info[i].write_np, O_WRONLY)) < 0)
            {
                perror("Cannot open write fifo at traverMonitor.c");
                deleteMonitorProcessInfo(proc_info, numMonitors);
                exit(EXIT_FAILURE);
            }

            // set poll events for file descriptors
            // 0 is read
            proc_info[i].file_descs[0].fd = proc_info[i].read_fd;
            proc_info[i].file_descs[0].events = POLLIN;

            // 1 is write
            proc_info[i].file_descs[1].fd = proc_info[i].write_fd;
            proc_info[i].file_descs[1].events = POLLOUT;

            // allocate memory of the hash table for the queries sent to this process
            proc_info[i].travel_req_queries = createHash(hashFunction, printHtCon, destroyHtCon, compareHtCon);

            // initialization is not yet finished, set isSet flag to false
            proc_info[i].isSet = false;
        }
    }
}

// initialize the pollfd struct inside the process holders struct mpi
void initPollfdStruct(mpi *proc_info, int numMonitors, struct pollfd *all_file_descs, int all_file_descs_size)
{
    // init the all file descriptors inside the poll fd struct
    // iterate through all the cells
    // the i'th and i'th + 1 cells of all_files_descs is related to a single cell in the mpi struct
    for(int i = 0; i < all_file_descs_size; i+=2)
    {
        // first initialize the read file descs
        // using i/FILE_DESCRIPTORS to calculate the correct cell of the mpi struct
        all_file_descs[i].fd = proc_info[i/FILE_DESCRIPTORS].file_descs[0].fd;
        all_file_descs[i].events = proc_info[i/FILE_DESCRIPTORS].file_descs[0].events;

        // second initialize the write file descs
        // using i + 1/FILE_DESCRIPTORS to calculate the correct cell of the mpi struct
        all_file_descs[i + 1].fd = proc_info[(i + 1)/FILE_DESCRIPTORS].file_descs[1].fd;
        all_file_descs[i + 1].events = proc_info[(i + 1)/FILE_DESCRIPTORS].file_descs[1].events;
    }

    return;
}

// reinitializes a dead monitor process without deleting needed fields or fifo's or hash tables
void reinitializeMonitor(mpi *proc_info, int numMonitors)
{
    pid_t pid;
    for(int i = 0; i < numMonitors; i++)
    {
        // checks if the process id of the the process holder is -1
        // this means that the process exited and we need to create a new one
        if (proc_info[i].mon_process_id == -1)
        {
            // close it's previous file descriptors
            // new connection will be established later
            close(proc_info[i].read_fd);
            close(proc_info[i].write_fd);

            // free the allocated memory of all the bloom filters if exists
            for (int j = 0; j < proc_info[i].blooms_amount; j++)
            {
                if (proc_info[i].blooms[j]) blmDestroy(proc_info[i].blooms[j]);
            }

            if (proc_info[i].blooms) free(proc_info[i].blooms);

            // free the allocated memory of the queries hash table is exists
            if (proc_info[i].travel_req_queries) destroyHash(proc_info[i].travel_req_queries);

            // set the file descriptors and the isSet variables to initial values, given by us
            proc_info[i].file_descs[0].fd = -1;
            proc_info[i].file_descs[1].fd = -1;
            proc_info[i].isSet = false;
        }
        else continue;

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
            execl("./Monitor", "./Monitor", proc_info[i].write_np, proc_info[i].read_np, NULL);
        }
        else
        {
            // inside parent process
            // set monitor process id
            proc_info[i].mon_process_id = pid;

            // open read and write fifos
            // read fifo
            if ((proc_info[i].read_fd = open(proc_info[i].read_np, O_RDONLY | O_NONBLOCK)) < 0)
            {
                perror("Cannot open read fifo at traverMonitor.c");
                deleteMonitorProcessInfo(proc_info, numMonitors);
                exit(EXIT_FAILURE);
            }

            // write fifo
            if ((proc_info[i].write_fd = open(proc_info[i].write_np, O_WRONLY)) < 0)
            {
                perror("Cannot open write fifo at traverMonitor.c");
                deleteMonitorProcessInfo(proc_info, numMonitors);
                exit(EXIT_FAILURE);
            }

            // set poll events for file descriptors
            // 0 is read
            proc_info[i].file_descs[0].fd = proc_info[i].read_fd;
            proc_info[i].file_descs[0].events = POLLIN;

            // 1 is write
            proc_info[i].file_descs[1].fd = proc_info[i].write_fd;
            proc_info[i].file_descs[1].events = POLLOUT;

            // allocate memory of the hash table for the queries sent to this process
            proc_info[i].travel_req_queries = createHash(hashFunction, printHtCon, destroyHtCon, compareHtCon);
        }
    }
}

// sending the buffer and the bloom filter size to all monitor processes
void sendBufferAndBloomSize(mpi *proc_info, int numMonitors, int buf_size, int blm_size, struct pollfd *all_file_descs)
{
    // first we're sending the buffer size, and the bloom filter size
    // transmission index is used to determine if we sent the buffer and bloom sizes to all the processes
    int transmision_index = 0;
    bool transmission_flag = false;

    // this bool array is used to "know" to which processes the sizes are already sent
    // initilize it with false since we've not sent anything yet
    bool sent[numMonitors];
    for(int i = 0; i < numMonitors; i++)
    {
        sent[i] = false;
    }

    // loop until message with code stop sent
    while (!transmission_flag)
    {
        // poll() syscall to find ready file descriptors
        int ready_fds = poll(all_file_descs, numMonitors * FILE_DESCRIPTORS, 0);

        // error checking
        if (ready_fds == -1)
        {
            perror("poll at monitor.c");
            exit(EXIT_FAILURE);
        }
        else if (ready_fds == 0) continue;

        // loop through the file descriptors stored in the pollfd struct
        // start from 1 and increment by 2, all the write fd's are stored in
        // odd cells, i.e. 1, 3, 5, ...
        for (int i = 1; i < numMonitors * FILE_DESCRIPTORS; i+=2)
        {
            // find out if the process has already received the sizes and continue if they did
            if (sent[i / FILE_DESCRIPTORS]) continue;

            // check if writing will not block
            if ((all_file_descs[i].revents & POLLOUT) == POLLOUT)
            {
                // send the buffer size, converting it to string first
                char *to_send = int_to_string(buf_size);

                // sending buffer size
                if (msg_send(all_file_descs[i].fd, MSG_BUFF_SIZE, to_send, buf_size) == EXIT_FAILURE)
                {
                    perror("msg_send at travelMonitor.c");
                    deleteMonitorProcessInfo(proc_info, numMonitors);
                    exit(EXIT_FAILURE);
                }

                // free allocated memory if exists
                if (to_send) free(to_send);

                // send the bloom filter size, converting it to string first
                to_send = int_to_string(blm_size);

                if (msg_send(all_file_descs[i].fd, MSG_BLM_SIZE, to_send, buf_size) == EXIT_FAILURE)
                {
                    perror("msg_send at travelMonitor.c");
                    deleteMonitorProcessInfo(proc_info, numMonitors);
                    exit(EXIT_FAILURE);
                }

                // free allocated memory if exists
                if (to_send) free(to_send);

                // send end transmission message
                if (msg_send(all_file_descs[i].fd, MSG_STOP_TRNS, STOP, buf_size) == EXIT_FAILURE)
                {
                    perror("msg_send at travelMonitor.c");
                    deleteMonitorProcessInfo(proc_info, numMonitors);
                    exit(EXIT_FAILURE);
                }

                // update transmision index
                transmision_index++;

                // update the sent array of the i'th process to true
                sent[i / FILE_DESCRIPTORS] = true;

                break;
            }
        }

        // check if every process has received the sizes and if true stop loop
        if (transmision_index == numMonitors)
        {
            transmission_flag = true;
        }
    }

    return;
}

// send the amount of subdirs each monitor process has
void sendSubDirsAmount(mpi *proc_info, int numMonitors, int buf_size, struct pollfd *all_file_descs)
{
    // transmission index is used to determine if we sent the subdirs amount to all the processes
    int transmision_index = 0;
    bool transmission_flag = false;

    // this bool array is used to "know" to which processes the sub directories amount are already sent
    // initilize it with false since we've not sent anything yet
    bool sent[numMonitors];
    for(int i = 0; i < numMonitors; i++)
    {
        sent[i] = false;
    }

    // loop until message with code stop sent
    while (!transmission_flag)
    {
        // poll() syscall to find ready file descriptors
        int ready_fds = poll(all_file_descs, numMonitors * FILE_DESCRIPTORS, 0);

        // error checking
        if (ready_fds == -1)
        {
            perror("poll at monitor.c");
            exit(EXIT_FAILURE);
        }
        else if (ready_fds == 0) continue;

        // loop through the file descriptors stored in the pollfd struct
        // start from 1 and increment by 2, all the write fd's are stored in
        // odd cells, i.e. 1, 3, 5, ...
        for (int i = 1; i < numMonitors * FILE_DESCRIPTORS; i+=2)
        {
            // find out if the process has already received the sizes and continue if they did
            if (sent[i / FILE_DESCRIPTORS]) continue;

            // check if writing will not block
            if ((all_file_descs[i].revents & POLLOUT) == POLLOUT)
            {
                // send amount of subdirs, convert it to string first
                char *to_send = int_to_string(proc_info[i / FILE_DESCRIPTORS].subdirsAmount);

                if (msg_send(all_file_descs[i].fd, MSG_AMNT_DIRS, to_send, buf_size) == EXIT_FAILURE)
                {
                    perror("msg_send at travelMonitor.c");
                    deleteMonitorProcessInfo(proc_info, numMonitors);
                    exit(EXIT_FAILURE);
                }

                // free allocated memory if exists
                if (to_send) free(to_send);

                // send transmission stop message
                if (msg_send(all_file_descs[i].fd, MSG_STOP_TRNS, STOP, buf_size) == EXIT_FAILURE)
                {
                    perror("msg_send at travelMonitor.c");
                    deleteMonitorProcessInfo(proc_info, numMonitors);
                    exit(EXIT_FAILURE);
                }

                // update transmision index
                transmision_index++;

                // update the sent array of the i'th process to true
                sent[i / FILE_DESCRIPTORS] = true;

                break;
            }
        }

        // check if every process has received the sizes and if true stop loop
        if (transmision_index == numMonitors)
        {
            transmission_flag = true;
        }
    }

    return;
}

// send the actual sub driectories paths
void sendSubDirs(mpi *proc_info, int numMonitors, int buf_size, struct pollfd *all_file_descs)
{
    // transmission index is used to determine if we sent the subdirs paths to all the processes
    int transmision_index = 0;
    bool transmission_flag = false;

    // this bool array is used to "know" to which processes the sub directories paths are already sent
    // initilize it with false since we've not sent anything yet
    bool sent[numMonitors];
    for(int i = 0; i < numMonitors; i++)
    {
        sent[i] = false;
    }

    // loop until message with code stop sent
    while (!transmission_flag)
    {
        // poll() syscall to find ready file descriptors
        int ready_fds = poll(all_file_descs, numMonitors * FILE_DESCRIPTORS, 0);

        // error checking
        if (ready_fds == -1)
        {
            perror("poll at monitor.c");
            exit(EXIT_FAILURE);
        }
        else if (ready_fds == 0) continue;

        // loop through the file descriptors stored in the pollfd struct
        // start from 1 and increment by 2, all the write fd's are stored in
        // odd cells, i.e. 1, 3, 5, ...
        for (int i = 1; i < numMonitors * FILE_DESCRIPTORS; i+=2)
        {
            // find out if the process has already received the sizes and continue if they did
            if (sent[i / FILE_DESCRIPTORS]) continue;

            // check if writing will not block
            if ((all_file_descs[i].revents & POLLOUT) == POLLOUT)
            {
                // send all the paths to the process
                for(int j = 0; j < proc_info[i / FILE_DESCRIPTORS].subdirsAmount; j++)
                {
                    if (msg_send(all_file_descs[i].fd, MSG_DIR, proc_info[i / FILE_DESCRIPTORS].subdirs[j], buf_size) == EXIT_FAILURE)
                    {
                        perror("msg_send at travelMonitor.c");
                        deleteMonitorProcessInfo(proc_info, numMonitors);
                        exit(EXIT_FAILURE);
                    }
                }

                // send stop transmission message
                if (msg_send(all_file_descs[i].fd, MSG_STOP_TRNS, STOP, buf_size) == EXIT_FAILURE)
                {
                    perror("msg_send at travelMonitor.c");
                    deleteMonitorProcessInfo(proc_info, numMonitors);
                    exit(EXIT_FAILURE);
                }

                // update transmision index
                transmision_index++;

                // update the sent array of the i'th process to true
                sent[i / FILE_DESCRIPTORS] = true;

                break;
            }
        }

        // check if every process has received the sizes and if true stop loop
        if (transmision_index == numMonitors)
        {
            transmission_flag = true;
        }
    }

    return;
}

// receive the initialized bloom filters from every process
void receiveBlooms(mpi *proc_info, int numMonitors, struct pollfd *all_file_descs, int all_file_descs_size, int buf_size)
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
        int ready_fds = poll(all_file_descs, numMonitors * FILE_DESCRIPTORS, 0);

        // error checking
        if (ready_fds == -1)
        {
            perror("poll at monitor.c");
            exit(EXIT_FAILURE);
        }
        else if (ready_fds == 0) continue;

        // receiving messages check only even indexes
        // i.e. 0, 2, 4, ...
        for(int i = 0; i < all_file_descs_size; i+=2)
        {
            // check if the process indicated from i is already set or not
            // and if it is continue
            if (proc_info[i/FILE_DESCRIPTORS].isSet == true) continue;

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
                        proc_info[i/FILE_DESCRIPTORS].blooms = malloc(atoi(buffer + max_header_digits) * sizeof(blm));

                        // check for correct memory allocation
                        assert(proc_info[i/FILE_DESCRIPTORS].blooms != null);

                        // set the amount of bloom filters
                        proc_info[i/FILE_DESCRIPTORS].blooms_amount = atoi(buffer + max_header_digits);
                        break;

                    // received a bloom filter
                    case MSG_BLM:

                        // get the string representation of the bloom filter
                        bloom_str = buffer + max_header_digits;

                        // decode the string to a bloom filter with the same fields
                        blm temp;
                        temp = string_to_blm(bloom_str);

                        // set the bloom filter in the blm's array and update the bloom_index array
                        proc_info[i/FILE_DESCRIPTORS].blooms[bloom_index[i/FILE_DESCRIPTORS]++] = temp;
                        break;

                    // received message stop transmission
                    case MSG_STOP_TRNS:

                        // set the current process isSet flag to true
                        proc_info[i/FILE_DESCRIPTORS].isSet = true;
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
