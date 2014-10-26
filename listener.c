/*
	listener.c
*/
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include "listener.h"

#define PORT_NUMBER 12345 
#define BUFFER_SIZE 10000 

void* Listener_thread()
{
	printf("Starting UDP listener...\n");

	int socket_fd;
	struct sockaddr_in name;

	char buffer[BUFFER_SIZE]; 

	// creating socket
	socket_fd = socket(PF_INET, SOCK_STREAM, 0);
	name.sin_family = AF_INET;
	name.sin_port = htons(PORT_NUMBER);
	bind(socket_fd, (struct sockaddr*) &name, sizeof(name));
	
	unsigned int length = sizeof(name);

	while (1) {
		recvfrom(socket_fd, buffer, BUFFER_SIZE, 0, 
			(struct sockaddr *) &name, &length);
		// send to processing buffer
	}

	close(socket_fd);
		
	return NULL;
}
