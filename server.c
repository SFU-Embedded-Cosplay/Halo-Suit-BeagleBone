/*
	server.c
	runs a server that listens to devices processes incoming information and sends responses
*/
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#include "listener.h"
#include "processor.h"

#define NUMBER_OF_THREADS 2
#define LISTENER_ID 1
#define PROCESSOR_ID 2

int main (int argc, char* argv[])
{
	printf("Powering up Mjonir systems...\n");

	pthread_t threads[NUMBER_OF_THREADS]; 
	
	pthread_create(&threads[LISTENER_ID], NULL, Listener_thread, NULL); 
	pthread_create(&threads[PROCESSOR_ID], NULL, Processor_thread, NULL);

	pthread_join(threads[LISTENER_ID], NULL);
	pthread_join(threads[PROCESSOR_ID], NULL);
	
	return 0;
}
