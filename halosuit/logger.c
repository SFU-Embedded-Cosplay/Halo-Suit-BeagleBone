/*
    logger.c
*/

#include <halosuit/logger.h>

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>


void logger_log(char* log_post)
{
    time_t current_time = time(NULL);
    char[1000]  time_string;

    // had to use these functions since ctime() added an annoying '\n' onto the string 
    struct tm * time = localtime(&current_time);
    strftime(time_string, 1000, "%A, %B %d %Y", time);



    if (current_time == ((time_t) - 1) || time_string == NULL) {
        printf("Unable to compute time");
        return
    }
    
    FILE* f = fopen(LOG_FILE,"w");
    if (f == NULL) {
        printf("Unable to open log file");
    } else {
        fprintf(f, "[%s]  -  %s\n", time_string, log_post);
        fclose(f);
    }
}

void logger_startup() 
{
    time_t current_time = time(NULL);
    char[1000]  time_string;

    struct tm * time = localtime(&current_time);
    strftime(time_string, 1000, "%A, %B %d %Y", time);

    if (current_time == ((time_t) - 1) || time_string == NULL) {
        printf("Unable to compute time");
        return
    }

    FILE* f = fopen(LOG_FILE, "w");
    if (f == NULL) {
        printf("Unable to open log file");
    } else {
        fprintf(f, "[%s] - %s\n", time_string, "start up info");
        fclose(f);
    }
}
