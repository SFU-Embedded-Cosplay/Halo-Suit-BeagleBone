/*
	processor.c
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "processor.h"
#include "queue.h"

#define MAX_MESSAGE_SIZE 10000 

static Queue incomingQueue = NULL;

void* Processor_thread() 
{
	incomingQueue = Queue_create();
	char* message = NULL;
	while(1)
	{
		Queue_dequeue(incomingQueue, &message);
		printf("Parsing %s", message);
		message = NULL;
	}
	return NULL;
}

void Processor_enqueue(char* message)
{
//	assert(strlen(message) < MAX_MESSAGE_SIZE);
	Queue_enqueue(incomingQueue, message);
}
