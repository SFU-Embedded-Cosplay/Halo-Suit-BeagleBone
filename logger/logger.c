/*
    logger.c
*/

#include <halosuit/logger.h>


void logger_log(char* log_post)
{
    time_t current_time;
    char* c_time_string;

    current_time = time(NULL);

    c_time_string = ctime(&current_time);

    if(current_time == ((time_t) - 1) || c_time_string == NULL) {
        printf("Unable to compute time");
    }
    
    FILE f* = fopen(LOGFILE,"w");
    if (f == NULL) {
        printf("Unable to open log file");
    }
    else {
        fprintf(f, "[%s]  -  %s\n", c_time_string, log_post);
        fclosef(f);
    }
}
