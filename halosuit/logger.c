/*
    logger.c
*/

#include <halosuit/logger.h>

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdarg.h>

void logger_log(const char *message, ...)
{
    /* compute current time */
    time_t current_time = time(NULL);
    char  time_string[1000];

    // had to use these functions since ctime() added an annoying '\n' onto the string
    struct tm * time = localtime(&current_time);
    // "%a, %d %b %Y %T %z" puts the time into RFC 2822 formatt
    strftime(time_string, 1000, "%a, %d %b %Y %T %z", time);

    if (current_time == ((time_t) - 1) || time_string == NULL) {
        printf("Unable to compute time");
        return;
    }

    /* print message to log file */
    FILE* log_file = fopen(LOG_FILE, "a");

    if (log_file == NULL) {
        printf("Unable to open log file");
    } else {
        va_list arguments;

        va_start(arguments, message);

        fprintf(log_file, "[%s]  -  ", time_string); // log current time
        vfprintf(log_file, message, arguments); // log actual message
        fprintf(log_file, "\n", time_string); // log new line for formatiing purposes

        va_end(arguments);

        fclose(log_file);
    }
}

void logger_startup()
{
    FILE* log_file = fopen(LOG_FILE, "a");
    if (log_file == NULL) {
        printf("Unable to open log file");
    } else {
        fprintf(log_file, "\n###########################################################\n");
        fclose(log_file);

        logger_log("Halosuit initializing");
    }
}
