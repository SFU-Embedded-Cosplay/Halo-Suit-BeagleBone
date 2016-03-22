/*
    logger.h
    provides functions to log strings into a log file
*/

#ifndef LOGGER
#define LOGGER

#define LOG_FILE "log.txt"
#define LOG_MAX_MESSAGE_LENGTH 1024

// takes a string and posts it in the log file
// void logger_log(char* log_post);
void logger_log(const char *message, ...);

// run at the beginning to indicate startup as well status information
void logger_startup();

#endif
