#include "chap03.h"

#include <ctype.h>
#include "connection_info.h"


void Service_ToUpper(char* arr, size_t len) {
	for (size_t i=0; i<len;++i) {
		if (arr[i] == '\0') break;
		arr[i] = toupper(arr[i]);
	}
}

int main() {

	// Create socket, bind and listen.

	// Maintain a linked list of connections.
	// Use select to listen to the one that needs serving.

	// Timeout and end if no connection in 5 minutes or 300 seconds.

	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_family = AF_INET;

	struct addrinfo *bind_address;
	if (getaddrinfo(0, "8080" ,&hints,&bind_address)) {
		fprintf(stderr, "getaddrinfo failed. %d\n", GETSOCKETERRNO());
		return 1;
	}


	SOCKET socket_listen;
	socket_listen = socket(bind_address->ai_family, bind_address->ai_socktype, bind_address->ai_protocol);
	if (!ISVALIDSOCKET(socket_listen)) {
		fprintf(stderr,"socket() failed. %d\n", GETSOCKETERRNO());
		return 1;
	}

	if(bind(socket_listen, bind_address->ai_addr, bind_address->ai_addrlen)) {
		fprintf(stderr,"bind() failed. %d\n", GETSOCKETERRNO());
		return 1;
	}

	if (listen(socket_listen, 10)) {
		fprintf(stderr,"listen() failed. %d\n", GETSOCKETERRNO());
		return 1;
	}

	printf("Listening...\n");

	ConnectionInfo clients_info = InitialiseConnectionInfo(socket_listen);

	struct timeval tv;
	tv.tv_sec = 300; // Wait for 5 mins.
    tv.tv_usec = 0; 

	while (1) {

		fd_set readfds_copy = clients_info.readfds;
		if (select(clients_info.max_socket+1, &readfds_copy, 0, 0, 0) < 0) {
			fprintf(stderr, "select() failed or timed out. %d", GETSOCKETERRNO());
			break;
		}

		// Handle new incoming comms.
		if (FD_ISSET(socket_listen, &readfds_copy)) {
			printf("New client connection!\n");
			struct sockaddr_storage client_address;
			socklen_t client_len = sizeof(client_address);

			SOCKET socket_client = accept(socket_listen, (struct sockaddr* )(&client_address),
	             (socklen_t *) &client_len);

			if (!ISVALIDSOCKET(socket_client)) {
				fprintf(stderr, "accept() failed. %d\n", GETSOCKETERRNO());
				return 1;
			}
			printf("Connection accepted.\n");

			char host[100];
			char serv[100];
			// Let's print out information about the client.
			if (getnameinfo((struct sockaddr* )&client_address, client_len, 
				host, sizeof(host),  serv, sizeof(serv), NI_NUMERICHOST)) {
				fprintf(stderr, "getnameinfo() failed. %d\n", GETSOCKETERRNO());
				return 1;
			}
			printf("New connection from : %s %s\n", host, serv);

			// Add to client list to listen for new messages.
			ConnectionInfoAppend (&clients_info, socket_client);

		}

		// Handle existing connections
		SockClientInfo* ptr = clients_info.client_info_start;
		while (ptr!= NULL) {
			if (FD_ISSET( ptr->socket, &readfds_copy)) {
				char msg[4096];
				const size_t msg_len = sizeof(msg)/sizeof(msg[0]);
				memset(msg, 0, msg_len);
				int bytes_received = recv(ptr->socket, msg, msg_len, 0);
				printf ("%d bytes received\n", bytes_received);

				if (bytes_received < 1) {
					fprintf(stderr, "Connection closed by client. %d\n", GETSOCKETERRNO());
					// remove connection from linked list.
					if(ConnectionInfoRemove(&clients_info, ptr)) {
						fprintf(stderr, "Error failed to remove closed connection from connection list.\n");
					}
					continue;
				}
				printf("Received message: \n %s\n", msg);

				// Perform Service.
				Service_ToUpper(msg, msg_len);

				printf("Sending response\n");
				int bytes_sent = send(ptr->socket, msg, msg_len, 0);

				printf("%d bytes sent\n", bytes_sent);
				printf("Sent message: \n %s \n", msg);

			}
			ptr = ptr->next;
		}

	}

	printf("Cleaning up all client info\n");
	CleanUpClientsInfo(&clients_info);
	printf("Closing listening socket\n");
	CLOSESOCKET(socket_listen);
	printf("Finished...\n");
	return 0;
}