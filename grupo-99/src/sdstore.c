#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>

#include "utils.h"
#include "communication.h"

/*
API:
$ ./sdstore
$ ./sdstore status
$ ./sdstore proc-file priority input-filename output-filename transformation-id-1 transformation-id-2 ...
Ex:
$ ./sdstore proc-file <priority> samples/file-a outputs/file-a-output bcompress nop gcompress encrypt nop
pending
processing
concluded (bytes-input: 2048, bytes-output: 1024)
*/
int main(int argc, char* argv[]) {
    char help_message[] = "./sdstore status\n"
        "./sdstore proc-file priority input-filename output-filename transformation-id-1 transformation-id-2 ...\n";

    signal(SIGINT, SIG_IGN);

    if (!(argc >= 2 && !strcmp(argv[1], "status")) && !(argc >= 5 && !strcmp(argv[1], "proc-file"))) {
        write(STDOUT_FILENO, help_message, sizeof(help_message));
        return EXIT_SUCCESS;
    }

    Comms msg = new_comms(getpid());

    // Create pipes
    exit_if((mkfifo(msg.pipe_s2c, 0666) != 0), "create server to client");
    exit_if((mkfifo(msg.pipe_c2s, 0666) != 0), "create client to server");

    // Open communication
    int auth = open(AUTH_PIPE, O_WRONLY);
    exit_if((auth == -1), "erro pipe auth");
    write(auth, &msg, sizeof(Comms));
    close(auth);

    // Relay command
    int out = open(msg.pipe_c2s, O_WRONLY);
    exit_if((out == -1), "erro pipe out");
    Cmd cmd = new_cmd(argc, argv);
    write(out, &cmd, sizeof(Cmd));
    close(out);

    // Receive response
    int in = open(msg.pipe_s2c, O_RDONLY);
    exit_if((in == -1), "erro pipe in");
    Response res;
    do {
        if(read(in, &res, sizeof(Response)))
            write(STDOUT_FILENO, res.line, res.sent);
    } while (res.sent);
    close(in);

    // Delete pipes
    unlink(msg.pipe_s2c);
    unlink(msg.pipe_c2s);

    return EXIT_SUCCESS;
}