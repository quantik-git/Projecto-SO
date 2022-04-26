#ifndef SDSTORED_H
#define SDSTORED_H

#include "task.h"
#include "transform.h"
#include "communication.h"

#define READ 0
#define WRITE 1

void status(char *pipe_in);
void proc_file(Comms msg, Cmd cmd);
void fork_off(Task task);
void apply_transf(Task task);
void exec_list(Task task);
void free_transf();

#endif