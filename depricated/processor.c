/*
	processor.c
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "processor.h"
#include "queue.h"
#include "json.h"

#define MAX_MESSAGE_SIZE 10000 
#define ERROR_STRING_SIZE 128

static Queue incomingQueue = NULL;

void* Processor_thread() 
{
	incomingQueue = Queue_create();
	json_value* object = NULL;
	char* message = NULL;
	int length = 0;

	
	while(1)
	{
		Queue_dequeue(incomingQueue, &message);
		printf("Parsing %s", message);
		object = json_parse(message, strlen(message));
		length = object->u.object.length;
		for (int i = 0; i < length; i++) {
			printf("%s\n", object->u.object.values[i].value->u.string.ptr);
		}
		message = NULL;
		object = NULL;
		length = 0;
	}
	return NULL;
}

void Processor_enqueue(char* message)
{
//	assert(strlen(message) < MAX_MESSAGE_SIZE);
	Queue_enqueue(incomingQueue, message);
}
