/*
    logger.h
    provides functions to log strings into a log file
*/

#ifndef LOGGER
#define LOGGER

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

#define LOG_FILE "log.txt"

// takes a string and posts it in the log file
void logger_log(char* log_post);

#endif
