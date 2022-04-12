#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define AUTH_PIPE "auth"
#define TERM_PIPE "term.txt"


// max pid 4194304
typedef struct auth {
    char pid[8];
    char pipe_s2c[16];
    char pipe_c2s[16];
} Auth;

typedef struct cmd {
    int argc;
    char argv[64][64];
} Cmd;

// 512 total
typedef struct response {
    int sent;
    char line[508];
} Response;

Auth auth_new(int pid);

Cmd cmd_new(int argc, char* argv[]);

Response new_res(char *line);

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