#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "loggerIntr.h"
#include "generalIntr.h"
#include "convertersIntr.h"
#include "ipc_protocolIntr.h"
#include "errorCheckingIntr.h"
#include "monitorServerHelpersIntr.h"

int argc_argv_manipulator_mon(int argc, char **argv, int *port, int *numThreads, int *s_buf_size, int *c_buf_size, int *blm_size, int *in_dirs_size, char ***input_directories_paths)
{
    // check if the arguments given are more than the programs name
    if(argc > 11)
    {
        // start reading from 1 because the first argv argument is the program name
        char *arg;
        bool correctArgs = false;

        *in_dirs_size = argc - 11;

        (*input_directories_paths) = calloc((argc - 11), sizeof(char *));

        for(int i = 1; i < 11; i++)
        {
            // getting the flag
            arg = argv[i];

            if(!strcmp(arg, "-p"))
            {
                // port number
                if (!isNumber(argv[i + 1], strlen(argv[i + 1]))) return EXIT_FAILURE;
                correctArgs = true;
                *port = atoi(argv[i + 1]);
            }
            else if(!strcmp(arg, "-t"))
            {
                // number of threads
                if (!isNumber(argv[i + 1], strlen(argv[i + 1]))) return EXIT_FAILURE;
                correctArgs = true;
                *numThreads = atoi(argv[i + 1]);
            }
            else if(!strcmp(arg, "-b"))
            {
                // socket buffer size
                if (!isNumber(argv[i + 1], strlen(argv[i + 1]))) return EXIT_FAILURE;
                correctArgs = true;
                *s_buf_size = atoi(argv[i + 1]);
            }
            else if(!strcmp(arg, "-c"))
            {
                // cyclic buffer size
                if (!isNumber(argv[i + 1], strlen(argv[i + 1]))) return EXIT_FAILURE;
                correctArgs = true;
                *c_buf_size = atoi(argv[i + 1]);
            }
            else if(!strcmp(arg, "-s"))
            {
                // bloom filter size
                if (!isNumber(argv[i + 1], strlen(argv[i + 1]))) return EXIT_FAILURE;
                correctArgs = true;
                *blm_size = atoi(argv[i + 1]);
            }
        }

        if (!correctArgs) return EXIT_FAILURE;

        // copying input directories paths
        for (int i = 11; i < argc; i++)
        {
            // allocate memory for the input direcotry path
            (*input_directories_paths)[i - 11] = calloc(strlen(argv[i]) + 1, sizeof(char));

            // check for correct memory allocation
            assert((*input_directories_paths)[i - 11] != null);

            // copy the path
            strcpy((*input_directories_paths)[i - 11], argv[i]);
        }

    }
    else return EXIT_FAILURE;

    // arguments read successfuly
    return EXIT_SUCCESS;
}

// initializes the container used in the monitor server
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

    // used to determine if the container passed the initialization segment
    (*container)->isSet = false;

    return;
}

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

// initialize container data structs to store the information from the files
void initDataStructs(mon_con *container, FILE **stream)
{
    // id fname lname country age virus status date
    //  0   1     2      3     4    5     6     7
    int size = 0;
    char *virus;
    char *str = null;
    char *country_to_insert = null;
    sList skip;
    blm bloom;
    pInfo person;
    personVac tempData = createPersonVaccinationData(null, true, null), toInsert;

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

        // if container is not set then we're initialize it
        // if not we're updating existing or adding new information
        if (!(*container)->isSet)
        {
            // new inserstion happens only if it is not a duplicate
            if(comparePersonInfo(person, input[1], input[2], country_to_insert, atoi(input[4]))
            && !(!strcmp(input[6], "NO") && size > 7)   // if the person is not vaccinated then no date is allowed after the answer NO
            && !(!strcmp(input[6], "YES") && size == 7) // if the person is vaccinated then a date is required after the answer YES
            && ((!vac && !notVac) || (!vacNode && !notVacNode))) // if the virus does not exist or if it exists but the person does not exists to a skip list for that virus
            {
                // id fname lname country age virus status date
                //  0   1     2      3     4    5     6     7

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
        }
        else if((*container)->isSet)
        {
            // check if the given country and virus are already in the right lists, and if there are update encounters variable
            if (!duplicateList((*container)->viruses, input[5])) insertList((*container)->viruses, createString(input[5]));

            // get the virus and country from each list to avoid data duplication, each virus and country are stored in the list
            // every other part of the code gets the pointer of the country or virus
            virus = getDataNode(findList((*container)->viruses, input[5]));

            // id fname lname country age virus status date
            //  0   1     2      3     4    5     6     7
            // check for vaccination status to insert him in the proper skip list and bloom filter
            if (!strcmp(input[6], "YES"))
            {
                // insert the vaccination on the bloom filter
                blm bloom = (blm)getEntry((*container)->bloom_filters, virus);

                // if it's a new virus create a bloom filter for it
                // else update old one
                if (!bloom)
                {
                    bloom = createFilter(virus, (*container)->bloom_size, hash_i);
                    blmInsert(bloom, getCitizenId(person));
                    insertHash((*container)->bloom_filters, bloom, (*container)->bloom_filters->hashFunc(virus));
                }
                else blmInsert(bloom, getCitizenId(person));

                // get the not vaccinated skip list
                sList skip_list_not_vac = (sList)getEntry((*container)->skip_lists_not_vaccinated, virus);

                // searching for the virus
                if(skip_list_not_vac)
                {
                    // if the person is previously not vaccinated delete the record
                    if ((toInsert = (personVac)getDataNode(skipListSearch(skip_list_not_vac, tempData))))
                    {
                        deleteFromSkipList(skip_list_not_vac, toInsert);
                    }
                }

                // check if a skip list for vaccinated people exists in the database, for the given virus
                // if not create one
                // if yes just insert the new record
                if(!getEntry((*container)->skip_lists_vaccinated, virus))
                {
                    sList skip_list = createSkipList(virus, destroyPersonVaccination, printPersonVaccinationData, comparePersonVaccinationData);
                    insertSkipList(skip_list, createPersonVaccinationData(person, true, input[7]));
                    insertHash((*container)->skip_lists_vaccinated, skip_list, (*container)->skip_lists_vaccinated->hashFunc(virus));
                }
                else
                {
                    insertSkipList(getEntry((*container)->skip_lists_vaccinated, virus), createPersonVaccinationData(person, true, input[7]));
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

        // deallocate memory used for the string, the parsed string, and the tempdata
        deleteParsedString(input, size);
        free(str);

    }

    // make the tempData point to null in order to not "lose" the last person stored
    tempData->person = null;

    // free allocated memory for the tempData if exists
    if (tempData) free(tempData);

    // last deletion of the string if it consumes memory
    if (str) free(str);

    return;
}

// open the sub directories read and initialize the data structs
char **getFilesPaths(char **input_dirs, int amount_of_subdirs, int *files_size)
{
    // how many files this process is monitoring
    *files_size = 0;

    // open each directory and initialize data structs
    for(int i = 0; i < amount_of_subdirs; i++)
    {
        // open the i'th sub directory
        struct dirent **in_dir;
        int amount_of_files = scandir(input_dirs[i], &in_dir, 0, alphasort);

        // update the files size variable, subtract 2 because of . and ..
        *files_size += (amount_of_files - 2);

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
            read_files[index] = calloc(strlen(in_dir[j]->d_name) + strlen(input_dirs[i]) + 2, sizeof(char));
            strcpy(read_files[index], input_dirs[i]);
            strcat(read_files[index], "/");
            strcat(read_files[index++], in_dir[j]->d_name);
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

    char *to_send = int_to_string(bloom_filters_amount);

    if (msg_send((*files_descs).fd, MSG_AMNT_BLM, to_send, buffer_size) == EXIT_FAILURE)
    {
        perror("msg_send at monitorServerHelpers.c : 510");
        exit(EXIT_FAILURE);
    }

    // free allocated memory of to_send if exists
    if (to_send) free(to_send);

    // send all the bloom filtres first
    for(int j = 0; j < bloom_filters_amount; j++)
    {

        if (msg_send((*files_descs).fd, MSG_BLM, blooms[j], buffer_size) == EXIT_FAILURE)
        {
            perror("msg_send at monitorServerHelpers.c : 523");
            exit(EXIT_FAILURE);
        }

        // free allocated memory for j'th bloom filters if exists
        if (blooms[j]) free(blooms[j]);
    }

    // free allocated memory of the bloom filters array if exists
    if (blooms) free(blooms);

    // send ending transmission message
    if (msg_send((*files_descs).fd, MSG_STOP_TRNS, STOP, buffer_size) == EXIT_FAILURE)
    {
        perror("msg_send at monitorServerHelpers.c : 537");
        exit(EXIT_FAILURE);
    }

    return;
}

void sendReadinessMessage(struct pollfd *files_descs, int buffer_size)
{
    // send message that the monitor server is ready to receive queries
    if (msg_send((*files_descs).fd, MSG_Q_READINESS, EMPTY, buffer_size) == EXIT_FAILURE)
    {
        perror("msg_send at monitorServerHelpers.c : 549");
        exit(EXIT_FAILURE);
    }

    // send ending transmission message
    if (msg_send((*files_descs).fd, MSG_STOP_TRNS, STOP, buffer_size) == EXIT_FAILURE)
    {
        perror("msg_send at monitorServerHelpers.c : 556");
        exit(EXIT_FAILURE);
    }

    return;
}

// produces the log file of the monitor server
void produce_log_mon(mon_con container, req_stats *stats, char *log_path)
{
    // get the countries monitored by this process
    int size = getListSize(container->countries);
    char **countries = calloc(size, sizeof(char *));

    // check for correct memory allocation
    assert(countries != null);

    // set the countries, copy the pointers since we're only allocating memory to temporalily store
    // them to an array used in create_log
    for(int i = 0; i < size; i++)
    {
        countries[i] = getDataNode(getListNodeAt(container->countries, i));
    }

    // create log of the process with given id, countries monitored, size and store it to log_path
    create_log(getpid(), countries, size, stats, log_path);

    // free allocated memory of the countries array if exists
    if (countries) free(countries);

    // free allocated memory of the container if exists
    destroyMonConData(container);

    return;
}

// delete the allocated memory of mon_con struct
void destroyMonConData(mon_con discard)
{
    // free every hash table and it's contents
    if (discard->persons) destroyHash(discard->persons);
    if (discard->bloom_filters) destroyHash(discard->bloom_filters);
    if (discard->skip_lists_vaccinated) destroyHash(discard->skip_lists_vaccinated);
    if (discard->skip_lists_not_vaccinated) destroyHash(discard->skip_lists_not_vaccinated);
    if (discard->viruses) destroyList(discard->viruses);
    if (discard->countries) destroyList(discard->countries);

    // free allocated memory of the monitor container if exists
    if (discard) free(discard);

    return;
}

// utility function used only by serverCleanup
// kills all the threads
static void kill_threads(pthread_t *threads, cb *circ_buf, int numThreads)
{
    // initialize the global variables and an array of string used to pass the ending message
    // to the threads in order for them to exit
    input_dir_paths_size = numThreads;

    // allocate memory
    char **des = calloc(numThreads, sizeof(char *));

    // check for correct memory allocation
    assert(des != null);

    // allocate and copy end message to the array of strings
    for(int i = 0; i < numThreads; i++)
    {
        // allocate memory
        des[i] = calloc(strlen("END") + 1, sizeof(char));

        // check for correct memory allocation
        assert(des[i] != null);

        // copy end message
        strcpy(des[i], "END");
    }

    // add the message end as many times as the number of threads, paths left is used as index
    for(paths_left = 0; paths_left < numThreads; paths_left++)
    {
        cb_add(*circ_buf, des[paths_left]);
        pthread_cond_signal(&buffer_not_empty);
    }

    // wait for all the threads to finish
    for(int i = 0; i < numThreads; i++)
    {
        pthread_join(threads[i], null);
    }

    // free allocated memory of the array of strings if exists
    for(int i = 0; i < numThreads; i++)
    {
        if (des[i]) free(des[i]);
    }

    if (des) free(des);

    return;
}

// clean up memory used by the server and produce the log file
void serverCleanup(int s_id, mon_con con, req_stats *stats, char *logs_path, pthread_t *threads, int numThreads, cb *circ_buf, char **files, int filesCount, char **paths, int pathsCount)
{
    // create the log file of the current monitor server process
    produce_log_mon(con, stats, logs_path);

    // kill all it's threads
    kill_threads(threads, circ_buf, numThreads);

    // delete the read file allocated memory of the string if exists
    for(int i = 0; i < filesCount; i++)
    {
        if(files[i]) free(files[i]);
    }

    if (files) free(files);

    // delete the input directories allocated memory of the string if exists
    for (int i = 0; i < pathsCount; i++)
    {
        if(paths[i]) free(paths[i]);
    }

    if (paths) free(paths);

    // destroy the conditional variables and the mutexes
    des_cond_mtx();

    // free allocated memory of the circural buffer if exists
    cb_destroy(*circ_buf);

    // close the socket id of the monitor server
    close(s_id);

    return;
}
