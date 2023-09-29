#ifndef HEADER_FILE
#define HEADER_FILE

#include <stdlib.h>
#include "chap03.h"

typedef struct sock_client_info
{
	struct sock_client_info* next;
	SOCKET socket;
} SockClientInfo;


typedef struct all_clients_info {
	fd_set readfds;
	SOCKET master;
	SOCKET max_socket;
	SockClientInfo* client_info_start;
} ConnectionInfo;

ConnectionInfo InitialiseConnectionInfo(SOCKET master) ;

void CleanUpClientsInfo(ConnectionInfo* clients_info) ;

void ConnectionInfoAppend (ConnectionInfo* clients_info, SOCKET socket) ;

int ConnectionInfoRemove (ConnectionInfo* clients_info, SockClientInfo* target) ;
#endif