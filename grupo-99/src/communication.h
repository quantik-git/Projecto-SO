#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <stdio.h>
#include <string.h>

#define AUTH_PIPE "auth"

// max pid 4194304
typedef struct communication {
    char pid[8];
    char pipe_s2c[16];
    char pipe_c2s[16];
} Comms;

typedef struct cmd {
    int argc;
    char argv[64][64];
} Cmd;

// 512 total
typedef struct response {
    int sent;
    char line[508];
} Response;

Comms new_comms(int pid);

Cmd new_cmd(int argc, char* argv[]);

Response new_res(char *line);

#endif