/* **************************************
 * mockBluetooth.c
 * emulates communication that would normally happen over bluetooth
 */
#ifdef MOCK_BLUETOOTH

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#include <beagleblue/beagleblue.h>

static char server_buffer[BUFFER_SIZE];

static pthread_t server_thread_id;

static void *server_thread();

bool is_listening;

int beagleblue_glass_send(char *buffer)
{
    // will do nothing 
    return 0;
}

int beagleblue_android_send(char *buffer)
{
    return 0;
}

void beagleblue_init(void (*on_receive)(char *buffer)) 
{
    pthread_create(&server_thread_id, NULL, &server_thread, on_receive);
}

void beagleblue_exit() 
{
    beagleblue_join();
}

void beagleblue_join()
{
    pthread_join(server_thread_id, NULL);
}

static void *server_thread(void *callback) 
{
    void (*on_receive)(char *) = callback;
    int sockfd; 
    int new_sockfd;
    socklen_t client_len;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    int port;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        // log or prinf
        printf("ERROR, bad socket");
        return NULL;
    } 

    server_addr.sin_family = AF_INET;  
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        printf("ERROR, binding error");
    }
    listen(sockfd, 1);
    client_len = sizeof(client_addr); 
    new_sockfd = accept(sockfd, (struct sockaddr *) &client_addr, &client_len);
    
    if (new_sockfd < 0) {
        printf("ERROR, on accept");
    }  
    is_listening = true;
    while (is_listening) {
        if (read(new_sockfd, server_buffer, sizeof(server_buffer)) < 0) {
            printf("ERROR, on read");
        }
        on_receive(server_buffer);
        memset(server_buffer,0, sizeof(server_buffer)); 
    } 
    close(sockfd); 
    return NULL;
}

#endif
