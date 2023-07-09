#include "connection_info.h"

ConnectionInfo InitialiseConnectionInfo(SOCKET master) {
	ConnectionInfo clients_info;
	clients_info.master = master;
	FD_ZERO(&clients_info.readfds);
	FD_SET(master, &clients_info.readfds);
	clients_info.client_info_start = NULL;
	clients_info.max_socket = master;
	return clients_info;
}

void CleanUpClientsInfo(ConnectionInfo* clients_info) {
	SockClientInfo* ptr = clients_info->client_info_start;
	while (ptr!= NULL) {
		SockClientInfo* ptr_temp = ptr;
		ptr = ptr->next; 
		free((void *)ptr_temp);
	}
}

void ConnectionInfoAppend (ConnectionInfo* clients_info, SOCKET socket) {
	// Algorithm assumes unique socket numbers are appended and so doesn't check for duplication.
	SockClientInfo*  new_tail;
	new_tail = (SockClientInfo*) malloc(sizeof(SockClientInfo));
	new_tail->socket = socket;
	new_tail->next = NULL;

	if (clients_info->client_info_start ==  NULL) {
		clients_info->client_info_start=new_tail;
	} else {
		SockClientInfo* ptr = clients_info->client_info_start;

		while (ptr->next!=NULL) {
			ptr = ptr->next;
		}
		ptr->next = new_tail;
	}
	
	FD_SET(socket, &clients_info->readfds);
	if (socket > clients_info->max_socket) {
		printf("Setting new max_socket: %d.\n", clients_info->max_socket);
		clients_info->max_socket = socket;
	}
}

int ConnectionInfoRemove (ConnectionInfo* clients_info, SockClientInfo* target) {
	// Note we are not reducing the max socket value for simplicity.
	if (target==NULL) return 1;

	SockClientInfo* ptr_temp = clients_info->client_info_start;
	SockClientInfo* prev = ptr_temp;
	while (!ptr_temp) {
		if (ptr_temp == target) {
			// target found

			if (ptr_temp == clients_info->client_info_start) { // Only happens at first.
					clients_info->client_info_start = ptr_temp->next;
			} else {
				prev->next = ptr_temp->next;
			}
			FD_CLR(target->socket, &clients_info->readfds);
			CLOSESOCKET(target->socket);
			free(ptr_temp);
			return 0;
		} else {
			prev = ptr_temp;
			ptr_temp = ptr_temp->next;
		}
	}

	return 1;
}