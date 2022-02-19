// file: networkIntr.h
// function declerations of the network functionalities
// of the server and client side of the application
// include guard
#pragma once

// headers below

// custom libs below
#include "ipc_protocolIntr.h"
#include "travelMonitorClientHelpersIntr.h"

// fucntion declarations
// general
char *get_local_ip(void);

// server side
int create_server_socket(char *ip, int port_num, struct sockaddr_in *addport);

// client side
int create_client_sockets(msi *proc_info, int num_monitorServers, char *ip);
void connect_to_servers(msi *proc_info, int num_monitorServers);