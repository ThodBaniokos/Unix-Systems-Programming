// file: network.c
// implementations of all the network related functions
// libraries below
#include <poll.h>
#include <stdio.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

// custom headers below
#include "networkIntr.h"
#include "convertersIntr.h"
#include "travelMonitorClientHelpersIntr.h"

// macro definitions below
#define BACKLOG 5

// fucntion implementations
// general
char *get_local_ip(void)
{
    char *ip = null;
    char hostrealname[256];
    struct hostent *host;

    // retrieve the name of the host machine
    if (gethostname(hostrealname, sizeof(hostrealname)) == -1)
    {
        perror("gethostname at network.c : 33");
        return null;
    }

    // get the hostent struct info from the machine's name
    if ((host = gethostbyname(hostrealname)) == null)
    {
        perror("gethostbyname at network.c : 40");
        return null;
    }

    // get the ip address of the machine
    ip = inet_ntoa(*((struct in_addr*) host->h_addr_list[0]));

    // return the ip found if its not null
    if (ip) return ip;

    return null;
}

// server side
// creates server socket
int create_server_socket(char *ip, int port_num, struct sockaddr_in *addport)
{
    // creating the socket used to read/write over
    int socket_id = socket(AF_INET, SOCK_STREAM, 0);

    // check if the server socket is successfuly created
    if (socket_id == -1)
    {
        perror("socket at network.c : 63");
        exit(EXIT_FAILURE);
    }

    // set the socket to be reusable
    int yes = 1;
    setsockopt(socket_id, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    // initialize the given sockaddr_in struct of the server
    memset(&(*addport), '0', sizeof((*addport)));

    // set the protocol family and the port used
    (*addport).sin_family = AF_INET;
    (*addport).sin_port = htons(port_num);

    // set the ip to be the given one and check for errors
    if (inet_pton(AF_INET, ip, &(*addport).sin_addr) <= 0)
    {
        perror("inet_pton at network.c : 81");
        close(socket_id);
        exit(EXIT_FAILURE);
    }

    // bind the socket id to the created info above
    if (bind(socket_id, (struct sockaddr *) &(*addport), sizeof((*addport))) < -1)
    {
        perror("bind at network.c : 89");
        exit(EXIT_FAILURE);
    }

    // listen for connections on this socket
    if (listen(socket_id, BACKLOG) < 0)
    {
        perror("listen network.c : 96");
        exit(EXIT_FAILURE);
    }

    // return the socket id to the calling function
    return socket_id;
}

// client side
// creates client sockets
int create_client_sockets(msi *proc_info, int num_monitorServers, char *ip)
{
    // for each monitor server we have, create a socket for communication
    for(int i = 0; i < num_monitorServers; i++)
    {
        // initialize fields to avoid possible bugs
        proc_info[i].c_socket_id = -1;
        memset(&proc_info[i].server_addr, '0', sizeof(proc_info[i].server_addr));

        // create the client socket
        if ((proc_info[i].c_socket_id = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            perror("socket at network.c : 118");
            return EXIT_FAILURE;
        }

        // initialize the sockaddr_in fields
        // set the protocol family
        proc_info[i].server_addr.sin_family = AF_INET;

        // set the port
        proc_info[i].server_addr.sin_port = htons(proc_info[i].port_num);

        // convert ip to bin net format and set the ip to the socket
        if (inet_pton(AF_INET, ip, &proc_info[i].server_addr.sin_addr) <= 0)
        {
            perror("inet_pton at network.c : 132");

            // close the sockets
            for(int j = 0; j < num_monitorServers; j++)
            {
                if (proc_info[j].c_socket_id != -1) close(proc_info[j].c_socket_id);
            }

            return EXIT_FAILURE;
        }

        // initialize the pollfd structs
        proc_info[i].socketfd.fd = proc_info[i].c_socket_id;
        proc_info[i].socketfd.events = POLLIN;
    }

    return EXIT_SUCCESS;
}

// server connection
void connect_to_servers(msi *proc_info, int num_monitorServers)
{
    // for all the processes
    for(int i = 0; i < num_monitorServers; i++)
    {
        int res;

        // try to connect untli the result is greater than zero, i.e. the connection was sucessful
        do
        {
            res = connect(proc_info[i].c_socket_id, (struct sockaddr *) &proc_info[i].server_addr, sizeof(proc_info[i].server_addr));
        } while (res < 0);
    }

    // connected to all servers
    return;
}
