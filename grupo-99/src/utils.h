#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

void exit_if(int cond, char* message) {
    if (cond) {
        perror(message);
        exit(EXIT_FAILURE);
    }
}

ssize_t read_segment(int fd, char *line, size_t size, char delimiter = '\n') {
    ssize_t i = 0;

    while(read(fd, &line[i], 1) && line[i++] != delimiter);

    return i;
}
