/* **********************************
 * beagleblue.h
 *
 */
#ifndef BEAGLEBLUE
#define BEAGLEBLUE

#define TIMEOUT 5 //represents a given timeout in seconds. timeouts occur on the send thread which forces a reconnect
#define BUFFER_SIZE 1024

int beagleblue_glass_send(char *); //sends char buffer with int specifying the number of characters returns the number of bytes sent
int beagleblue_android_send(char *);
void beagleblue_init(void (*on_receive)(char *)); //takes callback function as value which gets performed when something is received
void beagleblue_exit(); //stops threads
void beagleblue_join(); //waits for threads to join

/* **********************************
 * Hey guys let me know what you think of this given functionality
 * I can easily change anything
 */

#endif
