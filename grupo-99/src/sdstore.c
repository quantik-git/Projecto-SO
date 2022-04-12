#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "utils.h"

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

    if (!(argc >= 2 && !strcmp(argv[1], "status")) && !(argc >= 6 && !strcmp(argv[1], "proc-file"))) {
        write(STDOUT_FILENO, help_message, sizeof(help_message));
        return EXIT_SUCCESS;
    }

    Auth msg = auth_new(getpid());

    // Create pipes
    exit_if((mkfifo(msg.pipe_s2c, 0666) != 0), "create s2c");
    exit_if((mkfifo(msg.pipe_c2s, 0666) != 0), "create c2s");

    // Open communication
    int auth = open(AUTH_PIPE, O_WRONLY);
    exit_if((auth == -1), "erro pipe auth");
    write(auth, &msg, sizeof(Auth));
    close(auth);
    printf("Authed %s\n", msg.pid);

    // Relay command
    int out = open(msg.pipe_c2s, O_WRONLY);
    exit_if((out == -1), "erro pipe out");
    Cmd cmd = cmd_new(argc-1, argv);
    write(out, &cmd, sizeof(Cmd));
    close(out);

    // Receive response
    int in = open(msg.pipe_s2c, O_RDONLY);
    exit_if((in == -1), "erro pipe in");
    Response res;
    do {
        read(in, &res, sizeof(Response));
        write(STDOUT_FILENO, res.line, res.sent);
    } while (res.sent);
    close(in);

    // Delete pipes
    unlink(msg.pipe_s2c);
    unlink(msg.pipe_c2s);

    return EXIT_SUCCESS;
}