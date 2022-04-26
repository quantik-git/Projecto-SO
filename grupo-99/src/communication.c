#include "communication.h"

Comms new_comms(int pid) {
    Comms msg;
    sprintf(msg.pid, "%d", pid);
    sprintf(msg.pipe_c2s, "c2s_%d", pid);
    sprintf(msg.pipe_s2c, "s2c_%d", pid);
    return msg;
}

Cmd new_cmd(int argc, char* argv[]) {
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