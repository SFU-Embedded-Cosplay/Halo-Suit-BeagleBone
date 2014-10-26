/*
	queue.c
*/

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

#define QUEUE_SIZE 100

typedef struct {
	char* queue[QUEUE_SIZE]; 
	int firstPosition;
	int lastPosition;
	pthread_mutex_t queueMutex;
	sem_t queueSemaphore;	
} circularQueue;

circularQueue Queue_create()
{
	circularQueue jobQueue;
	pthread_mutex_init(&jobQueue.queueMutex, NULL);
	sem_init(&jobQueue.queueSemaphore, 0, 0);
	jobQueue.firstPosition = 0;
	jobQueue.lastPosition = 0;
	
	return jobQueue;
}

void Queue_enqueue(circularQueue* jobQueue, char* element) 
{
	int semValue = 0;
	pthread_mutex_lock(&jobQueue->queueMutex);
	{		
		sem_getvalue(&jobQueue->queueSemaphore, &semValue);
		if (semValue < QUEUE_SIZE) {	
			jobQueue->queue[jobQueue->lastPosition] = element;
			jobQueue->lastPosition = (jobQueue->lastPosition + 1) % QUEUE_SIZE;
			sem_post(&jobQueue->queueSemaphore);
		}
		else {
			printf("Queue is full, dumping element");
		}
	}
	pthread_mutex_unlock(&jobQueue->queueMutex);
} 

void Queue_dequeue(circularQueue* jobQueue, char** element)
{
	sem_wait(&jobQueue->queueSemaphore);
	pthread_mutex_lock(&jobQueue->queueMutex);
	{
		*element = jobQueue->queue[jobQueue->firstPosition];
		jobQueue->queue[jobQueue->firstPosition] = NULL;
		jobQueue->firstPosition = (jobQueue->firstPosition + 1) % QUEUE_SIZE;
	}
	pthread_mutex_unlock(&jobQueue->queueMutex);
}
