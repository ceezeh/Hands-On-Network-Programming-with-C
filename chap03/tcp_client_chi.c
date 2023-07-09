#include "chap03.h"


int main (int argc, char** argv) {
	if (argc < 3) {
		fprintf(stderr, " Usage: %s hostname port\n", argv[0]);
		return 1;
	}

	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = SOCK_STREAM;
	struct addrinfo* peer_address;

	if (getaddrinfo(argv[1], argv[2], &hints, &peer_address) ) {
		fprintf(stderr,"getaddrinfo() failed, %d\n", GETSOCKETERRNO());
		return 1;
	}

	printf("Remote address is: ");
	char remote_address[100];
	char service_name[100];
	getnameinfo(peer_address->ai_addr, peer_address->ai_addrlen,
		remote_address, sizeof(remote_address), 
		service_name, sizeof (service_name),
		NI_NUMERICHOST);
	printf("The remote address is %s %s\n", remote_address, service_name);

	SOCKET socket_peer;
	socket_peer = socket(peer_address->ai_family, peer_address->ai_socktype, peer_address->ai_protocol);
	if (!ISVALIDSOCKET(socket_peer)) {
		fprintf(stderr, "socket() failed, %d\n", GETSOCKETERRNO());
		return 1;
	} 
	printf("Connecting...\n");
	if (connect(socket_peer, peer_address->ai_addr, peer_address->ai_addrlen)) {
			fprintf(stderr, "connect() failed, %d\n", GETSOCKETERRNO());
		return 1;
	} 
	freeaddrinfo(peer_address);

	printf("Connected...\n");
	printf("To send data, enter text followed by enter.\n");
	while(1) {
		fd_set rfds;
		struct timeval tv;

		FD_ZERO(&rfds);
		FD_SET(0, &rfds); // stdin
		FD_SET(socket_peer, &rfds);
		// Wait up to 100ms
		tv.tv_sec = 0;
    	tv.tv_usec = 100,000;

    	if (select(socket_peer+1, &rfds, NULL, NULL, &tv) < 0) {
    		fprintf(stderr, "select() failed. %d\n", GETSOCKETERRNO());
    		return 1;
    	}

    	if (FD_ISSET(socket_peer, &rfds)) {
    		char msg[4096];
    		const size_t msg_len = sizeof(msg)/sizeof(msg[0]);
    		int bytes_received  = recv(socket_peer, msg, msg_len, 0);
    		if (bytes_received< 1) {
    			printf("Connections closed by client.\n");
    			break;
    		}

    		printf("Message received: ...\n");
    		printf("%s\n", msg);
    	}

    	char msg[4096];
    	const size_t msg_len = sizeof(msg)/sizeof(msg[0]);
		memset(msg, 0, msg_len);
    	if (FD_ISSET(0, &rfds)) {
    		if (!fgets(msg, msg_len, stdin)) break;
    		// if (msgs[0] == '\n') {}
    		printf("Sending...");
    		int bytes_sent = send(socket_peer, msg, msg_len, 0);
    		printf("Sent %d bytes.\n", bytes_sent);
    	}
	}
	
	printf("Closing socket...\n");
	CLOSESOCKET(socket_peer);
	printf("Finished.\n");
	return 0;
}