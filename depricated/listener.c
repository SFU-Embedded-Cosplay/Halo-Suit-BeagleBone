/*
	listener.c
*/
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include "listener.h"
#include "processor.h"

#define PORT_NUMBER 12345 
#define MESSAGE_SIZE 512

void* Listener_thread()
{
	printf("Starting UDP listener...\n");

	int socket_fd;
	struct sockaddr_in name;
	struct sockaddr_in client;

	char buffer[MESSAGE_SIZE]; 

	// creating socket
	socket_fd = socket(PF_INET, SOCK_DGRAM, 0);
	name.sin_port = htons(PORT_NUMBER);
	bind(socket_fd, (struct sockaddr*) &name, sizeof(name));
	
	unsigned int length = sizeof(name);
	while (1) {
		recvfrom(socket_fd, buffer, MESSAGE_SIZE, 0, 
			(struct sockaddr*) &client, &length);
		Processor_enqueue(buffer);
	}

	close(socket_fd);
		
	return NULL;
}
