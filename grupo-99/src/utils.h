#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#define AUTH_PIPE "auth"

void exit_if(int cond, char* message);

ssize_t read_segment(int fd, char *line, size_t size, char delimiter);