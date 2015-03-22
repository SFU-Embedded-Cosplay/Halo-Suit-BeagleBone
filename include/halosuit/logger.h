/*
    logger.h
    provides functions to log strings into a log file
*/

#ifndef LOGGER
#define LOGGER

#define LOG_FILE "log.txt"

// takes a string and posts it in the log file
void logger_log(char* log_post);

// run at the beginning to indicate startup as well status information
void logger_startup();

#endif
