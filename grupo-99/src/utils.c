#include "utils.h"

Auth auth_new(int pid) {
    Auth msg;
    sprintf(msg.pid, "%d", pid);
    sprintf(msg.pipe_c2s, "c2s_%d", pid);
    sprintf(msg.pipe_s2c, "s2c_%d", pid);
    return msg;
}

Cmd cmd_new(int argc, char* argv[]) {
    Cmd cmd;
    cmd.argc = argc;
    for (int i = 0; i < argc; i++)
        strcpy(cmd.argv[i], argv[i+1]);
    return cmd;
}

Response new_res(char *line) {
    Response res;
    res.sent = strlen(line);
    strcpy(res.line, line);
    return res;
}

void exit_if(int cond, char* message) {
    if (cond) {
        perror(message);
        exit(EXIT_FAILURE);
    }
}

ssize_t read_segment(int fd, char *line, size_t size, char delimiter) {
    ssize_t i = 0;

    while(read(fd, &line[i], 1) && line[i++] != delimiter);

    return i;
}