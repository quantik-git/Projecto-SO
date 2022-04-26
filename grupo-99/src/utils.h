#ifndef UTILS_H
#define UTILS_H

#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void exit_if(int cond, char* message);

ssize_t read_segment(int fd, char *line, size_t size, char delimiter);



// Code save
typedef enum status {
    PENDING = 0,
    PROCESSING = 1,
} STATUS_CODE;

static const char * const STATUS_TEXT[] = {
    [PENDING] = "Pending",
    [PROCESSING] = "Processing"
};

#endif