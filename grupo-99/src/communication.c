#include "communication.h"

Comms new_comms(int pid) {
    Comms msg;
    sprintf(msg.pid, "%d", pid);
    sprintf(msg.pipe_c2s, "./tmp/c2s_%d", pid);
    sprintf(msg.pipe_s2c, "./tmp/s2c_%d", pid);
    return msg;
}

Cmd new_cmd(int argc, char* argv[]) {
    Cmd cmd;
    if (!strcmp(argv[1], "proc-file")) {
        if (!strcmp(argv[2], "-p")) {
            cmd.argc = argc-2;
            int c = 1;
            for (int i = 0; i < cmd.argc; i++) {
                if (i == 1) c++;
                strcpy(cmd.argv[i], argv[i+c]);
            }
        } else {
            cmd.argc = argc;
            int c = 1;
            for (int i = 0; i < cmd.argc; i++) {
                if (i == 1) {
                    strcpy(cmd.argv[i], "0");
                    i++;
                    c--;
                }
                strcpy(cmd.argv[i], argv[i+c]);
            }
        }
    } else {
        cmd.argc = argc-1;
        for (int i = 0; i < cmd.argc; i++)
            strcpy(cmd.argv[i], argv[i+1]);
    }

    return cmd;
}

Response new_res(char *line) {
    Response res;
    res.sent = strlen(line);
    strcpy(res.line, line);
    return res;
}