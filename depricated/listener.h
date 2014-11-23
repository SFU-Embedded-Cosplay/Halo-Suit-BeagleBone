/*
	listener.h
	waits for incoming communication and then send it to processingBuffer
	used for testing real version will use bluetooth RFCOMM
*/

#ifndef LISTENER
#define LISTENER

// creates a listener for UDP communication
void* Listener_thread();

#endif
