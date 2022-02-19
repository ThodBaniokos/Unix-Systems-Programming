// file: monitorHelpers.c
// implementations of the helper functions
// libraries below
#include <poll.h>
#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <dirent.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/fcntl.h>

// custom headers below
#include "generalIntr.h"
#include "convertersIntr.h"
#include "ipc_protocolIntr.h"
#include "monitorHelpersIntr.h"
#include "mon_sig_actionsIntr.h"

// personVac related functinos
// create an instance of this struct
personVac createPersonVaccinationData(pInfo person, bool isVaccinated, char *date)
{
    // allocate memory for the struct
    personVac data = malloc(sizeof(*data));

    // error handler
    assert(data != null);

    // copy the person given, to avoid data duplication because the person already exists in our app
    if (person) data->person = person;
    else data->person = null;

    // set if the person is vaccinated or not
    data->isVaccinated = isVaccinated;

    // date is given only if the above is true but we're checking for both to avoid errors
    if (date && data->isVaccinated) { data->date = malloc((strlen(date) + 1) * sizeof(char)); assert(data->date != null); strcpy(data->date, date); }
    else data->date = null;

    return data;
}

// int functions
// comapres the person vaccination data struct with the given id
// the identifier of the struct is the person citizen id
int comparePersonVaccinationData(Item data, Item id)
{
    int num1 = atoi(((personVac)data)->person->citizenID);
    int num2 = atoi(((personVac)id)->person->citizenID);
    return num1 - num2;
}

// compares the person info with the given one
int comparePersonInfo(Item person, char *firstName, char *lastName, char *country, unsigned short int age)
{
    // id is the same because of the hash table insertion
    return (!strcmp(((pInfo)person)->firstName, firstName) // first name must be the same
        && !strcmp(((pInfo)person)->lastName, lastName)  // last name must be the same
        && !strcmp(((pInfo)person)->country, country) // country must be the same
        && ((pInfo)person)->age == age);    // age must be the same
}


// reads command line arguments if given
// ./Monitor ./tmp/main_proc_write_1 ./tmp/child_proc_write_1
int argc_argv_manipulator_mon(int argc, char **argv, char **read_path, char **write_path)
{
    // check if the arguments given are more than the programs name
    if(argc == 3)
    {
        // opening read and write fifo's
        // first argument is the read fifo, second is the write
        *read_path = argv[1];
        *write_path = argv[2];
    }
    else return EXIT_FAILURE;

    // arguments read successfuly
    return EXIT_SUCCESS;
}

// received sigusr1, reading the directories and finding the new files, update the data structs with the new information
int updateDataStructs(mon_con *container, char **input_dirs, int amount_of_sub_dirs, char **read_files, int amount_of_files)
{
    // loop through the amount of subdirectories monitored by the monitor process
    for(int i = 0; i < amount_of_sub_dirs; i++)
    {
        // scan the i'th input directory
        struct dirent **in_dir;
        FILE *stream;
        int files = scandir(input_dirs[i], &in_dir, 0, alphasort);

        // starting from 2 to avoid ., .. folders
        for(int j = 2; j < files; j++)
        {
            // check if the file exists in the directory
            if (isFilePresent(read_files, amount_of_files, in_dir[j]->d_name)) continue;

            // if not build the full path of the file, i.e. project_folder/input_directory/Country/Country-X.txt
            char fullpath[strlen(input_dirs[i]) + strlen(in_dir[j]->d_name) + 2];
            strcpy(fullpath, input_dirs[i]);
            strcat(fullpath, "/");
            strcat(fullpath, in_dir[j]->d_name);

            // open the file
            stream = fopen(fullpath, "r");

            // check if the file is opened properly
            assert(stream != NULL);

            // re-initialize the data structs
            initDataStructs(container, &stream, in_dir[j]->d_name);

            // close the file to avoid memory leaks
            fclose(stream);
        }

        // free the allocated memory of the dirent struct if exists
        for(int j = 0; j < files; j++)
        {
            if (in_dir[j]) free(in_dir[j]);
        }

        if (in_dir) free(in_dir);
    }

    return EXIT_SUCCESS;
}

// void functions
// prints the person vaccination data information
void printPersonVaccinationData(Item data)
{
    printf("%s : ", ((personVac)data)->person->citizenID);
    if (((personVac)data)->isVaccinated) printf("VACCINATED ON %s\n", ((personVac)data)->date);
    else printf("NOT VACCINATED\n");
}

// deletes allocated memory of the person vaccination data
void destroyPersonVaccination(Item discard)
{
    // if there is a date given delete it
    if (((personVac)discard)->date) free(((personVac)discard)->date);

    // delete the pointer
    free(discard);

    return;
}

// initialization of the program
void monitor_initialization(char *write_p, char *read_p, int *write_fd, int *read_fd, struct pollfd *files_descs, mon_con *container, char ***input_dirs, char ***read_files, int *amount_of_sub_dirs, int *amount_of_files, int *buffer_size, int *blm_size)
{
    // open named pipes and init pollfd struct
    initPollfdSturctMon(write_p, read_p, write_fd, read_fd, files_descs);

    // reading bloom filter and buffer size
    readBufferAndBloomSize(buffer_size, blm_size, files_descs);

    // reading amount of subdirs of the monitor process
    *amount_of_sub_dirs = 0;

    readSubdirsAmount(amount_of_sub_dirs, *buffer_size, files_descs);

    // reading subdirs
    (*input_dirs) = readSubDirs(*amount_of_sub_dirs, *buffer_size, files_descs);

    // init datastructs, open directories and start inserting data
    (*container) = malloc(sizeof(**container));

    // check for correct memory allocation
    assert(container != null);

    initContainer(container, *blm_size);
    (*read_files) = openSubDirs(container, *input_dirs, *amount_of_sub_dirs, amount_of_files);

    // sending amount of bloom filters and the bloom filters at the main process
    sendBlooms(container, files_descs, *buffer_size, *blm_size);

    return;
}

// general initialization of the monitor process needed fields, signal handlers, etc.
void mon_general_initialization(void)
{
    signal(SIGUSR1, sigusr1_handler);
    signal(SIGINT, sigint_handler);
    signal(SIGQUIT, sigquit_handler);
    set_max_int_digits();
    set_max_header_digits();
}

// initialize the container with all the data structs inside
void initContainer(mon_con *container, int bloom_size)
{
    // hash table used to store all the bloom filters
    (*container)->bloom_filters = createHash(hashFunction, null, blmDestroy, compareBloomKey);

    // hash table used to store the skip list of the vaccinated persons
    (*container)->skip_lists_vaccinated = createHash(hashFunction, printSkipList, destroySkipList, compareSkipListKey);

    // hash table used to store the skip list of the not vaccinated persons
    (*container)->skip_lists_not_vaccinated = createHash(hashFunction, printSkipList, destroySkipList, compareSkipListKey);

    // hash table used to store all the persons
    (*container)->persons = createHash(hashFunction, printPersonInformation, deletePerson, compareCitizenId);

    // list where the viruses are going to be stored
    (*container)->viruses = createList(null, compareStrings, printString, deletePointer);

    // list where the countries are going to be stored
    (*container)->countries = createList(null, compareStrings, printString, deletePointer);

    // set the bloom size
    (*container)->bloom_size = bloom_size;

    return;
}

// initialize container data structs to store the information from the files
void initDataStructs(mon_con *container, FILE **stream, char *countryfile)
{
    // id fname lname country age virus status date
    //  0   1     2      3     4    5     6     7
    int size = 0, errors = 0;
    char *str = null;
    char *country_to_insert = null;
    pInfo person;
    personVac tempData = createPersonVaccinationData(null, true, null);

    while (*stream != null)
    {
        // read file line by line
        str = makeString(stream);

        // error handling if statement
        if (!strlen(str)) break;

        // parse line
        char **input = stringParser(str, &size, " ");


        // check if we've already read this country
        // and get it from the countries list
        // if not store it in the countries list
        if (!duplicateList((*container)->countries, input[3]))
        {
            // allocate memory for the string
            country_to_insert = malloc((strlen(input[3]) + 1) * sizeof(char));

            // check for correct memory allocation
            assert(country_to_insert != null);

            // copy the string contents
            strcpy(country_to_insert, input[3]);

            // insert the country string in the list
            insertList((*container)->countries, country_to_insert);
        }
        else country_to_insert = (char *)getDataNode(findList((*container)->countries, input[3]));

        // try to find the person with id : citizenID
        person = getDataNode(doesKeyExists((*container)->persons, input[0]));

        // if the person does not exist create and insert him in the hashtable
        if(!person)
        {
            // create person
            person = createPerson(input[0], input[1], input[2], country_to_insert, (unsigned short int)atoi(input[4]));

            // insert the person in the hash table
            insertHash((*container)->persons, person, (*container)->persons->hashFunc(getCitizenId(person)));
        }

        // temp data to make comparisons, copying the person pointer to avoid unnessecary mallocs
        tempData->person = person;

        // get the skip lists associated with the virus
        sList vac = (sList)getEntry((*container)->skip_lists_vaccinated, input[5]), notVac = (sList)getEntry((*container)->skip_lists_not_vaccinated, input[5]);
        lNode vacNode, notVacNode;

        // check if the virus exist and if yes search for the person
        // if not set everything to null
        if (vac) vacNode = skipListSearch(vac, tempData);
        else vacNode = null;

        if (notVac) notVacNode = skipListSearch(notVac, tempData);
        else notVacNode = null;

        // inserstion happens only if it is not a duplicate
        if(comparePersonInfo(person, input[1], input[2], country_to_insert, atoi(input[4]))
        && !(!strcmp(input[6], "NO") && size > 7)   // if the person is not vaccinated then no date is allowed after the answer NO
        && !(!strcmp(input[6], "YES") && size == 7) // if the person is vaccinated then a date is required after the answer YES
        && ((!vac && !notVac) || (!vacNode && !notVacNode))) // if the virus does not exist or if it exists but the person does not exists to a skip list for that virus
        {
            // id fname lname country age virus status date
            //  0   1     2      3     4    5     6     7
            sList skip;
            blm bloom;
            char *virus;

            // check if the given country and virus are already in the right lists, and if there are update encounters variable
            if (!duplicateList((*container)->viruses, input[5])) insertList((*container)->viruses, createString(input[5]));

            // get the virus and country from each list to avoid data duplication, each virus and country are stored in the list
            // every other part of the code gets the pointer of the country or virus
            virus = getDataNode(findList((*container)->viruses, input[5]));

            // check for vaccination status to insert him in the proper skip list and bloom filter
            if (!strcmp(input[6], "YES"))
            {
                // if the citizen id is new create a new skip list and hash entry
                if (!doesKeyExists((*container)->skip_lists_vaccinated, virus))
                {
                    // create a new skip list and bloom filter, the given virus did not exist in the app until now
                    skip = createSkipList(virus, destroyPersonVaccination, printPersonVaccinationData, comparePersonVaccinationData);
                    bloom = createFilter(virus, (*container)->bloom_size, hash_i);

                    // insert the data in the two data structs
                    insertSkipList(skip, createPersonVaccinationData(person, true, input[7]));
                    blmInsert(bloom, getCitizenId(person));

                    // insert the two data structs (pointers) in the proper hash tables
                    insertHash((*container)->skip_lists_vaccinated, skip, (*container)->skip_lists_vaccinated->hashFunc(virus));
                    insertHash((*container)->bloom_filters, bloom, (*container)->bloom_filters->hashFunc(virus));
                }
                else
                {
                    // if not just insert the person in the correct hash entry of the virus
                    insertSkipList(getEntry((*container)->skip_lists_vaccinated, virus), createPersonVaccinationData(person, true, input[7]));
                    blmInsert(getEntry((*container)->bloom_filters, virus), getCitizenId(person));
                }
            }
            else
            {
                // if the citizen id is new create a new skip list and hash entry
                if (!doesKeyExists((*container)->skip_lists_not_vaccinated, virus))
                {
                    // create only a new skip list for the not vaccinated person and repeat the same steps as above
                    skip = createSkipList(virus, destroyPersonVaccination, printPersonVaccinationData, comparePersonVaccinationData);
                    insertSkipList(skip, createPersonVaccinationData(person, false, null));
                    insertHash((*container)->skip_lists_not_vaccinated, skip, (*container)->skip_lists_not_vaccinated->hashFunc(virus));
                }
                else
                {
                    // if not just insert the person in the correct hash entry of the virus
                    insertSkipList(getEntry((*container)->skip_lists_not_vaccinated, virus), createPersonVaccinationData(person, false, null));
                }
            }
        }
        else
        {
            // update the encountered errors counter
            errors++;
        }

        // deallocate memory used for the string, the parsed string, and the tempdata
        deleteParsedString(input, size);
        free(str);

    }

    // print amount of errors encountered by each monitor process, with it's pid
    if (errors) printf("[%u] - [%s] - ERRORS ENCOUNTERED %d.\n", getpid(), countryfile, errors);

    // make the tempData point to null in order to not "lose" the last person stored
    tempData->person = null;

    // free allocated memory for the tempData if exists
    if (tempData) free(tempData);

    // last deletion of the string if it consumes memory
    if (str) free(str);

    return;
}

// initilaze pollfd struct of the monitor process
void initPollfdSturctMon(char *write_p, char *read_p, int *write_fd, int *read_fd, struct pollfd *files_descs)
{
    // open write only named pipe
    if ((*write_fd = open(write_p, O_WRONLY)) < 0)
    {
        perror("Cannot open write fifo at monitor.c");
        exit(EXIT_FAILURE);
    }

    // open read only named pipe
    if((*read_fd = open(read_p, O_RDONLY | O_NONBLOCK)) < 0)
    {
        perror("Cannot open read fifo at monitor.c");
        exit(EXIT_FAILURE);
    }

    // set the file descriptors and events in the pollfd struct
    // 0 is read
    files_descs[0].fd = *read_fd;
    files_descs[0].events = POLLIN;

    // 1 is write
    files_descs[1].fd = *write_fd;
    files_descs[1].events = POLLOUT;

    return;
}

// read buffer and bloom filter size in the monitor process
void readBufferAndBloomSize(int *buffer_size, int *blm_size, struct pollfd *files_descs)
{
    char *buffer;
    bool transmission_flag = false;

    // loop until message with code stop received
    while (!transmission_flag)
    {
        // poll() syscall to find ready file descriptors
        int ready_fds = poll(files_descs, FILE_DESCRIPTORS, 0);

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
            // check if the events returned from poll() is POLLIN, i.e. there's data to read
            if ((files_descs[i].revents & POLLIN) == POLLIN)
            {
                // the first read of the monitor process is the buffer size
                // since there's no given one from the travelMonitor process we temporalily set the
                // buffer size as the int max length to read the integer given from the main process
                // and the we're setting the buffer size to the one given and then using that buffer
                // size throughout the program
                if (*buffer_size == 0) *buffer_size = snprintf(NULL, *buffer_size, "%d", INT_MAX);

                // we're reading ints so the buffer size needed for ints is fixed
                if ((buffer = msg_read(files_descs[i].fd, *buffer_size)) == NULL)
                {
                    perror("msg_read at monitor.c");
                    exit(EXIT_FAILURE);
                }

                // switch case to find out what message we received from the message code
                switch (get_msg_code(buffer))
                {
                    // buffer size message
                    case MSG_BUFF_SIZE:
                        *buffer_size = atoi(buffer + max_header_digits);
                        break;

                    // bloom size message
                    case MSG_BLM_SIZE:
                        *blm_size = atoi(buffer + max_header_digits);
                        break;

                    // stop transmission message
                    case MSG_STOP_TRNS:
                        transmission_flag = true;
                        break;
                }

                // free the allocated memory of the buffer, if exists
                if (buffer) free(buffer);
            }
        }
    }

    return;
}

// read the amount of sub directories monitored by the monitor process
void readSubdirsAmount(int *amount_of_sub_dirs, int buffer_size, struct pollfd *files_descs)
{
    char *buffer;
    bool transmission_flag = false;

    // loop until message with code stop received
    while (!transmission_flag)
    {
        // poll() syscall to find ready file descriptors
        int ready_fds = poll(files_descs, FILE_DESCRIPTORS, 0);

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
            // check if the events returned from poll() is POLLIN, i.e. there's data to read
            if ((files_descs[i].revents & POLLIN) == POLLIN)
            {
                if ((buffer = msg_read(files_descs[i].fd, buffer_size)) == NULL)
                {
                    perror("msg_read at monitor.c");
                    exit(EXIT_FAILURE);
                }

                // switch case to find out what message we received from the message code
                switch (get_msg_code(buffer))
                {
                    // amount of subdirs message
                    case MSG_AMNT_DIRS:
                        *amount_of_sub_dirs = atoi(buffer + max_header_digits);
                        break;

                    // stop transmission message
                    case MSG_STOP_TRNS:
                        transmission_flag = true;
                        break;
                }

                // free allocated memory of the buffer if exists
                if (buffer) free(buffer);
            }
        }
    }

    return;
}

// read the actual input sub directories in the monitor process
char **readSubDirs(int amount_of_subdirs, int buffer_size, struct pollfd *files_descs)
{
    // array index to store each subdirectory we're reading to the array below
    int index = 0;

    // array to store the amount of subdirs
    char **input_dirs = calloc(amount_of_subdirs, sizeof(char *));

    // check for correct memory allocation
    assert(input_dirs != null);

    char *buffer;
    bool transmission_flag = false;

    // loop until message with code stop received
    while (!transmission_flag)
    {
        // poll() syscall to find ready file descriptors
        int ready_fds = poll(files_descs, FILE_DESCRIPTORS, 0);

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
            // check if the events returned from poll() is POLLIN, i.e. there's data to read
            if ((files_descs[i].revents & POLLIN) == POLLIN)
            {
                if ((buffer = msg_read(files_descs[i].fd, buffer_size)) == NULL)
                {
                    perror("msg_read at monitor.c");
                    exit(EXIT_FAILURE);
                }

                // switch case to find out what message we received from the message code
                switch (get_msg_code(buffer))
                {
                    // message with input directory in monitor process
                    case MSG_DIR:
                        // allocate memory for the string storing the subdirectory name
                        input_dirs[index] = malloc((strlen(buffer + max_header_digits) + 1) * sizeof(char));

                        // check for correct memory allocation
                        assert(input_dirs[index] != null);

                        // copy the string sent from the main process
                        strcpy(input_dirs[index++], buffer + max_header_digits);
                        break;

                    // stop transmission message
                    case MSG_STOP_TRNS:
                        transmission_flag = true;
                        break;
                }

                // free allocated memory of the buffer if exists
                if (buffer) free(buffer);
            }
        }
    }

    // return the input directories read from the main process
    return input_dirs;
}

// open the sub directories read and initialize the data structs
char **openSubDirs(mon_con *container, char **input_dirs, int amount_of_subdirs, int *files_size)
{
    // how many files this process is monitoring
    *files_size = 0;

    // open each directory and initialize data structs
    for(int i = 0; i < amount_of_subdirs; i++)
    {
        // open the i'th sub directory
        struct dirent **in_dir;
        FILE *stream;
        int amount_of_files = scandir(input_dirs[i], &in_dir, 0, alphasort);

        // update the files size variable, subtract 2 because of . and ..
        *files_size += (amount_of_files - 2);

        // starting from 2 to avoid ., .. folders
        for(int j = 2; j < amount_of_files; j++)
        {
            // build the full path of the file, i.e. project_folder/input_directory/Country/Country-X.txt
            char fullpath[strlen(input_dirs[i]) + strlen(in_dir[j]->d_name) + 2];
            strcpy(fullpath, input_dirs[i]);
            strcat(fullpath, "/");
            strcat(fullpath, in_dir[j]->d_name);

            // open the file
            stream = fopen(fullpath, "r");

            // check if the file opened correctly
            assert(stream != NULL);

            // initialize data structs with the contents of the files
            initDataStructs(container, &stream, in_dir[j]->d_name);

            // close the file
            fclose(stream);
        }

        // free allocated memory of the dirent struct if exists
        for(int j = 0; j < amount_of_files; j++)
        {
            if (in_dir[j]) free(in_dir[j]);
        }

        if (in_dir) free(in_dir);
    }

    // copy the files read in this monitor process
    int index = 0;

    // allocate memory for the read files array
    char **read_files = calloc(*files_size, sizeof(char *));

    // check for correct memory allocation
    assert(read_files != null);

    // copy the names of the files with the same technique as above
    for(int i = 0; i < amount_of_subdirs; i++)
    {
        struct dirent **in_dir;
        int amount_of_files = scandir(input_dirs[i], &in_dir, 0, alphasort);

        for(int j = 2; j < amount_of_files; j++)
        {
            read_files[index] = calloc(strlen(in_dir[j]->d_name) + 1, sizeof(char));
            assert(read_files[index] != null);
            strcpy(read_files[index++], in_dir[j]->d_name);
        }

        for(int j = 0; j < amount_of_files; j++)
        {
            if (in_dir[j]) free(in_dir[j]);
        }

        if (in_dir) free(in_dir);
    }

    return read_files;
}

// send the bloom filters to the main process
void sendBlooms(mon_con *container, struct pollfd *files_descs, int buffer_size, int blm_size)
{
    // getting the amount of viruses found in all the input files of the monitor process
    // and then we're using this to create an array to store the bloom filters to string
    // there are at most viruses_amount of bloom filters
    int viruses_amount = getListSize((*container)->viruses);

    // building bloom filters to string
    char **blooms = malloc(viruses_amount * sizeof(char *));

    // check for correct memory allocation
    assert(blooms != null);
    int bloom_filters_amount = 0;

    for(int i = 0; i < viruses_amount; i++)
    {
        // get the virus from the viruses list
        char *blm_key = getDataNode(getListNodeAt((*container)->viruses, i));

        // search in the hash table that the bloom filters are stored for the
        // bloom filter with the blm_key
        blm temp = getEntry((*container)->bloom_filters, blm_key);

        // if there's no bloom filter with this key continue
        if (!temp) continue;

        // if there is then create a string bloom filter representation and store
        // it in the array created above, while updating the bloom_filters_amount counter
        blooms[bloom_filters_amount++] = blm_to_string(temp, blm_size);

        // if we've reached the viruses_amount then break
        if (bloom_filters_amount == viruses_amount) break;
    }

    // sending the bloom filters to the main process
    int transmission_index = 1;
    bool transmission_flag = false;

    // loop until message with code stop sent
    while (!transmission_flag)
    {
        // poll() syscall to find ready file descriptors
        int ready_fds = poll(files_descs, FILE_DESCRIPTORS, 0);

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
            if ((files_descs[i].revents & POLLOUT) == POLLOUT)
            {
                // if the transmision index is 1 then send the amount of bloom filters first
                // if it is 2 then send all the bloom filters and then the message stop
                if(transmission_index == 1)
                {
                    char *to_send = int_to_string(bloom_filters_amount);

                    if (msg_send(files_descs[i].fd, MSG_AMNT_BLM, to_send, buffer_size) == EXIT_FAILURE)
                    {
                        perror("msg_send at monitor.c");
                        exit(EXIT_FAILURE);
                    }

                    // free allocated memory of to_send if exists
                    if (to_send) free(to_send);

                    // update transmission index
                    transmission_index++;
                    break;
                }
                else if(transmission_index == 2)
                {
                    // send all the bloom filtres first
                    for(int j = 0; j < bloom_filters_amount; j++)
                    {

                        if (msg_send(files_descs[i].fd, MSG_BLM, blooms[j], buffer_size) == EXIT_FAILURE)
                        {
                            perror("msg_send at monitor.c");
                            exit(EXIT_FAILURE);
                        }

                        // free allocated memory for j'th bloom filters if exists
                        if (blooms[j]) free(blooms[j]);
                    }

                    // free allocated memory of the bloom filters array if exists
                    if (blooms) free(blooms);

                    // send ending transmission message
                    if (msg_send(files_descs[i].fd, MSG_STOP_TRNS, STOP, buffer_size) == EXIT_FAILURE)
                    {
                        perror("msg_send at monitor.c");
                        exit(EXIT_FAILURE);
                    }

                    transmission_flag = true;
                }
            }
        }
    }

    return;
}

// delete the allocated memory of mon_con struct
void destroyMonConData(mon_con discard)
{
    // free every hash table and it's contents
    destroyHash(discard->persons);
    destroyHash(discard->bloom_filters);
    destroyHash(discard->skip_lists_vaccinated);
    destroyHash(discard->skip_lists_not_vaccinated);
    destroyList(discard->viruses);
    destroyList(discard->countries);

    // free allocated memory of the monitor container if exists
    if (discard) free(discard);

    return;
}
