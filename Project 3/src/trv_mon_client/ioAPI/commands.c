// file: commands.c
// this file contains the implementation of the commands for mngstd
// libraries below
#include <poll.h>
#include <time.h>
#include <wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

// custom headers below
#include "blmIntr.h"
#include "utilsIntr.h"
#include "loggerIntr.h"
#include "commonIntr.h"
#include "commandsIntr.h"
#include "hashTableIntr.h"
#include "linkedListIntr.h"
#include "convertersIntr.h"
#include "ipc_protocolIntr.h"
#include "serializationIntr.h"

// utility functions used for the queries in travelmonitor
// binary search in the countries array to find the process we need
static int searchCountry(msi *proc_info, int numMonitors, const char *country);

static int searchCountry(msi *proc_info, int numMonitors, const char *country)
{
    // loop through all the monitor processes to find the right one
    // i.e. the monitor process monitoring the given country
    for(int i = 0; i < numMonitors; i++)
    {
        int l = 0, r = proc_info[i].subdirsAmount - 1;

        while (l <= r)
        {
            int mid = l + (r - l) / 2;

            int val = strcmp(proc_info[i].countries[mid], country);

            if (!val) return i;
            else if (val < 0) l = mid + 1;
            else r = mid - 1;
        }
    }

    return -1;
}

// checks if the given date is between the lower and upper bound, used to calculate the
// query statistics in order to make the travel stats sequence work correctly
static int checkDateRange(char *lowerDate, char *upperDate, char *date);

static int checkDateRange(char *lowerDate, char *upperDate, char *date)
{
    int lowerSize, upperSize, datesize, flag = 1;
    char **lowerBound = stringParser(lowerDate, &lowerSize, "-");
    char **upperBound = stringParser(upperDate, &upperSize, "-");
    char **requestDate = stringParser(date, &datesize, "-");

    if ((atoi(requestDate[2]) >= atoi(lowerBound[2]) || atoi(requestDate[2]) <= atoi(upperBound[2])) && // check for year
    (atoi(requestDate[1]) >= atoi(lowerBound[1]) || atoi(requestDate[1]) <= atoi(upperBound[1])) && // check for month
    (atoi(requestDate[0]) >= atoi(lowerBound[0]) || atoi(requestDate[0]) <= atoi(upperBound[0]))) // check for date
    {
        flag = 0;
    }

    deleteParsedString(lowerBound, lowerSize);
    deleteParsedString(upperBound, upperSize);
    deleteParsedString(requestDate, datesize);

    return flag;
}

// commands in travelmonitor
// queries sequencies
// travel reuqest sequence
int trvl_request_sequence(msi *proc_info, int numMonitors, char **input, int inputSize, int buf_size, req_stats *stats, struct pollfd *all_file_descs, int pollfdSize)
{
    // /travelRequest citizenID date countryFrom countryTo virusName
    //        0           1       2        3         4          5
    int index = searchCountry(proc_info, numMonitors, input[3]);
    char *buffer;
    char *res = null;
    bool bloom_flag = false;

    // error in utility function
    if (index == -1)
    {
        printf("ERROR : NON EXISTING COUNTRY IN DATABASE\n");
        return EXIT_SUCCESS;
    }

    // first check in the bloom filter
    for(int i = 0; i < proc_info[index].blooms_amount; i++)
    {
        // find the bloom filter monitoring the given virus
        if (!compareBloomKey(proc_info[index].blooms[i], input[5]))
        {
            // check if the person is not vaccinated
            if (!blmLookup(proc_info[index].blooms[i], input[1]))
            {
                printf("REQUEST REJECTED - YOU ARE NOT VACCINATED\n");

                // update the statistics to produce the log file correclty
                (*stats).rejected++;
                (*stats).total++;

                // used to avoid the operations after this one
                bloom_flag = true;
            }
            break;
        }
    }

    // if the person is "maybe" vaccinated send a message to the monitor process and find if
    // the person is 100% vaccinated to the given virus
    if (!bloom_flag)
    {

        // make the query a single string
        char *query = serialize_query(input, inputSize);

        if (msg_send(all_file_descs[index].fd, MSG_CMD_TR_RQ, query, buf_size) == EXIT_FAILURE)
        {
            perror("msg_send at commands.c : 128");
            return EXIT_FAILURE;
        }

        // if transmission index is 0 send message stop transmission
        if (msg_send(all_file_descs[index].fd, MSG_STOP_TRNS, STOP, buf_size) == EXIT_FAILURE)
        {
            perror("msg_send at commands.c : 135");
            return EXIT_FAILURE;
        }

        // free allocated memory if exists
        if (query) free(query);

        // reset the transmision flag, used again in the receiving end
        bool transmission_flag = false;

        // loop till the message stop is received
        while (!transmission_flag)
        {
            // poll() syscall to find the ready file descs
            int ready_fds = poll(all_file_descs, pollfdSize, 0);

            // error checking
            if (ready_fds == -1)
            {
                perror("poll at commands.c : 154");
                return EXIT_FAILURE;
            }
            else if (ready_fds == 0) continue;

            // check if there's data to read
            if ((all_file_descs[index].revents & POLLIN) == POLLIN)
            {
                // read the message
                if ((buffer = msg_read(all_file_descs[index].fd, buf_size)) == NULL)
                {
                    perror("msg_read at commands.c : 165");
                    return EXIT_FAILURE;
                }

                // decode message
                switch (get_msg_code(buffer))
                {
                    // received answet to thre travel request query
                    case MSG_ANS_TR_RQ:
                        printf("%s\n", buffer + max_header_digits);

                        // check if the answer was possitive or negative
                        res = strstr(buffer + max_header_digits, "REQUEST ACCEPTED");
                        break;

                        // received message stop transmission
                    case MSG_STOP_TRNS:
                        transmission_flag = true;
                        break;
                }

                // free allocated memory if exists
                if (buffer) free(buffer);
            }
            else continue;
        }

        // check the result and update the right varialbe in the stats struct
        if (res) (*stats).accepted++;
        else (*stats).rejected++;

        // update the total queries given
        (*stats).total++;
    }

    // /travelRequest citizenID date countryFrom countryTo virusName
    //        0           1       2        3         4          5

    index = searchCountry(proc_info, numMonitors, input[4]);

    // initialize the container stored in the travel_req_queries hash table of proc_info to null
    ht_con hash_container = null;

    // find the right container, determined by countryTo
    hash_container = (ht_con)getEntry(proc_info[index].travel_req_queries, input[4]);

    // if there's no container create one
    if (!hash_container)
    {
        // allocate memory for the container
        ht_con ins = malloc(sizeof(*ins));

        // check for correct memory allocation
        assert(ins != null);

        // allocate memory for the country
        ins->country = calloc((strlen(input[4]) + 1), sizeof(char));

        // check for correct memory allocation
        assert(ins->country != null);

        // copy the country
        strcpy(ins->country, input[4]);

        // create a list to store lists for all viruses
        ins->viruses = createList(null, compareListKey, printList, destroyList);

        // create a list for the specified virus
        lList tempList = createList(input[5], compareDates, printDates, destroyDates);

        // allocate memory for the request information of the specified date
        req_info to_insert = malloc(sizeof(*to_insert));

        // check for correct memory allocation
        assert(to_insert != null);

        // allocate memory for the date string
        to_insert->date = calloc((strlen(input[2]) + 1), sizeof(char));

        // check for correct memory allocation
        assert(to_insert->date != null);

        // initialize the request info struct
        to_insert->accepted = 0;
        to_insert->rejected = 0;
        strcpy(to_insert->date, input[2]);

        // the bloom filter flag is false
        // check if the result is negative or possitive
        // and update the correct variable
        if (!bloom_flag)
        {
            if (res) to_insert->accepted++;
            else to_insert->rejected++;
        }

        // insert everything to the req_info struct in the list
        // the list in the viruses list of the container
        // and the container to the hash table
        insertList(tempList, to_insert);
        insertList(ins->viruses, tempList);
        insertHash(proc_info[index].travel_req_queries, ins, proc_info[index].travel_req_queries->hashFunc(ins->country));
    }
    else
    {
        // find the list of the specified virus
        lList virusList;
        virusList = (lList)getDataNode(findList(hash_container->viruses, input[5]));

        // if the lists exists
        if (virusList)
        {
            // check if the date is given again
            if (duplicateList(virusList, input[2]))
            {
                // update the correct variable
                req_info node = (req_info)getDataNode(findList(virusList, input[2]));
                if (!bloom_flag)
                {
                    if (res) node->accepted++;
                    else node->rejected++;
                }
            }
            else
            {
                // if the date is not given again create a new req info variable like described above
                req_info to_insert = malloc(sizeof(*to_insert));

                // check for correct memory allocation
                assert(to_insert != null);

                to_insert->date = calloc((strlen(input[2]) + 1), sizeof(char));

                // check for correct memory allocation
                assert(to_insert->date != null);

                to_insert->accepted = 0;
                to_insert->rejected = 0;
                strcpy(to_insert->date, input[2]);

                if (!bloom_flag)
                {
                    if (res) to_insert->accepted++;
                    else to_insert->rejected++;
                }

                insertList(virusList, to_insert);
            }
        }
        else
        {
            // if the list does not exist create a new one like desribed above and do the same operations
            lList tempList = createList(input[5], compareDates, printDates, destroyDates);

            req_info to_insert = malloc(sizeof(*to_insert));

            // check for correct memory allocation
            assert(to_insert != null);

            to_insert->date = calloc((strlen(input[2]) + 1), sizeof(char));

            // check for correct memory allocation
            assert(to_insert->date != null);

            to_insert->accepted = 0;
            to_insert->rejected = 0;
            strcpy(to_insert->date, input[2]);

            if (!bloom_flag)
            {
                if (res) to_insert->accepted++;
                else to_insert->rejected++;
            }

            insertList(tempList, to_insert);
            insertList(hash_container->viruses, tempList);
        }
    }

    // successful execution
    return EXIT_SUCCESS;
}

// travel stats sequence
int trvl_stats_sequence(msi *proc_info, int numMonitors, struct pollfd *all_file_descs, int pollfd_size, char **input, int inputSize, int buf_size)
{
    // /travelStats virusName date1 date2 [country]
    //      0           1       2     3       4
    int index = -1;

    // if a country is given find it
    if (inputSize == 5) index = searchCountry(proc_info, numMonitors, input[4]);

    // if there's no country given print the statistics for all the countries checking for the specified virus
    if (index == -1)
    {
        // iterate through all the stored countries in the proc_info
        for(int i = 0; i < numMonitors; i++)
        {
            ht_con hash_container;

            // check for all countries
            for(int j = 0; j < proc_info[i].subdirsAmount; j++)
            {
                // quries list
                lList queries;

                // temporary auxiliary variable
                req_info temp;

                // counters to calculate statistics
                int total, accepted, rejected, list_index;
                total = accepted = rejected = list_index = 0;

                // find the correct container
                hash_container = (ht_con)getEntry(proc_info[i].travel_req_queries, proc_info[i].countries[j]);

                // if there's no country associated with a container in the hash table
                // this country have not received a travel request yet, continue
                if (!hash_container) continue;

                // get the queries list of the specified virus
                queries = (lList)getDataNode(findList(hash_container->viruses, input[1]));

                // if there's no queries list, this country does not check for the specified virus, continue
                if (!queries) continue;
                else
                {
                    // get all the nodes and calculate if the travel date is between the given date range
                    while ((temp = getDataNode(getListNodeAt(queries, list_index++))) != null)
                    {
                        // copy contents to avoid data deletion by accident
                        char *tempDate = calloc(strlen(temp->date) + 1, sizeof(char));
                        assert(tempDate != null);
                        strcpy(tempDate, temp->date);

                        // if the travel date is between the given date range update the statistics counters
                        if(!checkDateRange(input[2], input[3], tempDate))
                        {
                            total += (temp->accepted + temp->rejected);
                            accepted += temp->accepted;
                            rejected += temp->rejected;
                        }

                        if (tempDate) free(tempDate);
                    }

                    // print results
                    printf("%s\n", proc_info[i].countries[j]);
                    printf("TOTAL REQUESTS %d\nACCEPTED %d\nREJECTED %d\n", total, accepted, rejected);
                    if (list_index < getListSize(queries)) printf("\n");
                }
            }
        }
    }
    else
    {
        // do the same thing as above but only for the specified country
        ht_con hash_container;
        lList queries;
        req_info temp;

        int total, accepted, rejected, list_index;
        total = accepted = rejected = list_index = 0;

        hash_container = (ht_con)getEntry(proc_info[index].travel_req_queries, input[4]);

        // if there's no container notify user and return
        if (!hash_container)
        {
            printf("%s DOES NOT HAVE TRAVEL REQUESTS YET\n", input[4]);
            return EXIT_SUCCESS;
        }

        queries = (lList)getDataNode(findList(hash_container->viruses, input[1]));

        // if there's no queries list for the specified virus notify user and return
        if (!queries)
        {
            printf("%s DOES NOT CHECK FOR THE SPECIFIED VIRUS\n", input[4]);
            return EXIT_SUCCESS;
        }
        else
        {
            while ((temp = getDataNode(getListNodeAt(queries, list_index++))) != null)
            {
                // copy contents to avoid data deletion by accident
                char *tempDate = calloc(strlen(temp->date) + 1, sizeof(char));
                assert(tempDate != null);
                strcpy(tempDate, temp->date);

                if(!checkDateRange(input[2], input[3], tempDate))
                {
                    total += (temp->accepted + temp->rejected);
                    accepted += temp->accepted;
                    rejected += temp->rejected;
                }

                // free allocated memory if exists
                if (tempDate) free(tempDate);
            }

            printf("%s\n",input[4]);
            printf("TOTAL REQUESTS %d\nACCEPTED %d\nREJECTED %d\n", total, accepted, rejected);
            if (list_index < getListSize(queries)) printf("\n");
        }
    }

    return EXIT_SUCCESS;
}

// add vaccination records query
int add_vac_recs_sequence(msi *proc_info, int numMonitors, char **input, int inputSize, int buf_size, struct pollfd *all_file_descs, int pollfdSize)
{
    // find the given country in the proc info array to "know" where to send the signal
    int index = searchCountry(proc_info, numMonitors, input[1]);

    // if no country was found notify user of the error and return
    if (index == -1)
    {
        printf("ERROR : NON EXISTING COUNTRY IN DATABASE\n");
        return EXIT_SUCCESS;
    }

    // send the message add vaccination records
    if (msg_send(all_file_descs[index].fd, MSG_CMD_ADD_VAC_REC, EMPTY, buf_size) == EXIT_FAILURE)
    {
        perror("msg_send at commands.c : 492");
        return EXIT_FAILURE;
    }

    if (msg_send(all_file_descs[index].fd, MSG_STOP_TRNS, STOP, buf_size) == EXIT_FAILURE)
    {
        perror("msg_send at commands.c : 498");
        return EXIT_FAILURE;
    }

    // delete the bloom filters memory if exists
    for(int i = 0; i < proc_info[index].blooms_amount; i++)
    {
        if (proc_info[index].blooms[i]) blmDestroy(proc_info[index].blooms[i]);
    }
    if (proc_info[index].blooms) free(proc_info[index].blooms);

    proc_info[index].isSet = false;
    proc_info[index].isReady = false;

    // receive bloom filters
    receiveBlooms(proc_info, numMonitors, all_file_descs, pollfdSize, buf_size);

    // ready to send queries
    readinessCheck(proc_info, numMonitors, buf_size, all_file_descs, numMonitors);

    return EXIT_SUCCESS;
}

// search vaccination status query
int search_vac_status_sequence(struct pollfd *all_file_descs, int numMonitors, int pollfd_size, char **input, int input_size, int buf_size)
{
    // /searchVaccinationStatus citizenID
    char *query = serialize_query(input, input_size);

    // used to determine if we've sent the query to every monitor process
    int transmission_index = 0;
    bool transmission_flag = false;

    for (int i = 0; i < pollfd_size; i++)
    {
        // send query to all monitors
        if (msg_send(all_file_descs[i].fd, MSG_CMD_SCH_VAC_ST, query, buf_size) == EXIT_FAILURE)
        {
            perror("msg_send at commands.c : 536");
            return EXIT_FAILURE;
        }

        // send stop transmission message
        if (msg_send(all_file_descs[i].fd, MSG_STOP_TRNS, STOP, buf_size) == EXIT_FAILURE)
        {
            perror("msg_send at commands.c : 543");
            return EXIT_FAILURE;
        }
    }

    // free allocated memory if exists
    if (query) free(query);

    char *buffer;

    // loop till all monitors sent an answer
    while (!transmission_flag)
    {
        // poll() syscall to determine ready fds
        int ready_fds = poll(all_file_descs, pollfd_size, 0);

        // error checking
        if (ready_fds == -1)
        {
            perror("poll at commands.c : 562");
            return EXIT_FAILURE;
        }
        else if (ready_fds == 0) continue;

        for (int i = 0; i < pollfd_size; i++)
        {
            // check if there's data to read
            if ((all_file_descs[i].revents & POLLIN) == POLLIN)
            {
                // read message
                if ((buffer = msg_read(all_file_descs[i].fd, buf_size)) == NULL)
                {
                    perror("msg_read at commands.c : 575");
                    return EXIT_FAILURE;
                }

                // decode message
                switch (get_msg_code(buffer))
                {
                    // received answer
                    case MSG_ANS_SCH_VAC_ST:
                        // print the answer
                        printf("%s\n", buffer + max_header_digits);
                        break;

                    // received stop transmission message
                    case MSG_STOP_TRNS:
                        transmission_index++;
                        break;
                }

                // free allocated memory if exists
                if (buffer) free(buffer);
            }
            else continue;
        }

        // if all monitor processes sent an answer stop the loop
        if (transmission_index == numMonitors) transmission_flag = true;
    }

    return EXIT_SUCCESS;

}

// exit query
int exit_sequence_query(msi *proc_info, int numMonitors, req_stats *stats, char *log_path, struct pollfd *all_file_descs, int pollfd_size, int buf_size)
{
    // send the exit query to all the servers
    for(int i = 0; i < numMonitors; i++)
    {
        // send query
        if (msg_send(proc_info[i].c_socket_id, MSG_CMD_EXIT, EMPTY, buf_size) == EXIT_FAILURE)
        {
            perror("msg_send at commands.c : 617");
            return EXIT_FAILURE;
        }

        // send stop transmission message
        if (msg_send(proc_info[i].c_socket_id, MSG_STOP_TRNS, STOP, buf_size) == EXIT_FAILURE)
        {
            perror("msg_send at commands.c : 624");
            return EXIT_FAILURE;
        }
    }

    // used to calculate the amount of countries existing in the application
    int counter = 0;

    // countries array
    char **countries;

    // send sigkill signal to all processes
    for (int i = 0; i < numMonitors; i++)
    {
        // update countries counter
        counter += proc_info[i].subdirsAmount;
    }

    // allocate memory for all the countries
    countries = calloc(counter, sizeof(char *));

    // check for correct memory allocation
    assert(countries != null);

    // copy all the countries to the countries array
    int index = 0;

    for (int i = 0; i < numMonitors; i++)
    {
        for(int j = 0; j < proc_info[i].subdirsAmount; j++)
        {
            countries[index++] = proc_info[i].countries[j];
        }
    }

    // create log file
    create_log(getpid(), countries, counter, stats, log_path);

    // free allocated memory if exists
    if (countries) free(countries);

    return EXIT_SUCCESS;
}
