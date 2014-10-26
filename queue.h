/*
	queue.h
*/
#ifndef QUEUE
#define QUEUE

// struct that holds the queue and all relative information
typedef struct circularQueue;

// initializes a circularQueue's array, positions indexs, mutex and semaphore
circularQueue Queue_create();

// adds a message to the back of the queue if full message is dropped
void Queue_enqueue(circularQueue jobQueue, char* element);

// takes the first element of the queue if empty thread sleeps
void Queue_dequeue(circularQueue jobQueue, char* element);

#endif
