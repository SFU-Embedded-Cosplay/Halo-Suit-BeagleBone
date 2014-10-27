/*
	processor.h
*/
#ifndef PROCESSOR_THREAD
#define PROCESSOR_THREAD

// processes messages from the processing buffer, sleeps when buffer is empty
void* Processor_thread();

// adds a message to the incoming queue
void Processor_enqueue(char* message);

#endif
