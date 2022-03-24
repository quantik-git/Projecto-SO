#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "utils.h"

/*
max pid 4194304

API:
$ ./sdstore
./sdstore status
./sdstore proc-file priority input-filename output-filename transformation-id-1 transformation-id-2 ...

$ ./sdstore proc-file <priority> samples/file-a outputs/file-a-output bcompress nop gcompress encrypt nop
pending
processing
concluded (bytes-input: 2048, bytes-output: 1024)
*/

void create_in(char *pid, char *pipe);
void create_out(char *pid, char *pipe);
void status();
void proc_file();


int main(int argc, char* argv[]) {
    char help_message[] = "./sdstore status\n"
        "./sdstore proc-file priority input-filename output-filename transformation-id-1 transformation-id-2 ...\n";

    if (argc >= 2 && !strcmp(argv[1], "status")) {
        status();
    } else if (argc >= 6 && !strcmp(argv[1], "proc-file")) {
        // o argc >= 6 inclui a priority
        proc_file();
    } else {
        write(STDOUT_FILENO, help_message, sizeof(help_message));
    }

    return EXIT_SUCCESS;
}


void create_in(char *pid, char *pipe) {
    sprintf(pipe, "in_%s", pid);
    exit_if((mkfifo(pipe, 0666) != 0), "create in");
}

void create_out(char *pid, char *pipe) {
    sprintf(pipe, "out_%s", pid);
    exit_if((mkfifo(pipe, 0666) != 0), "create out");
}

void status() {
    char id[16];
    sprintf(id, "%d", getpid());

    char pipe_out[32], pipe_in[32];
    create_in(id, pipe_in);
    create_out(id, pipe_out);

    int auth = open(AUTH_PIPE, O_WRONLY);
    exit_if((auth == -1), "erro pipe auth");

    // AUTH do cliente
    write(auth, id, strlen(id));
    close(auth);

    printf("here %s\n", id);

    int out = open(pipe_out, O_WRONLY);
    exit_if((out == -1), "erro pipe out");

    write(out, "status", sizeof("status"));
    close(out);

    int in = open(pipe_in, O_RDONLY);
    exit_if((in == -1), "erro pipe in");

    printf("here\n");

    char buffer[4];
    while (read(in, buffer, 1) > 0)
        write(STDOUT_FILENO, buffer, 1);

    close(in);
    unlink(pipe_in);
    unlink(pipe_out);
}

void proc_file() {
    return;
}