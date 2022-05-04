#ifndef SDSTORED_H
#define SDSTORED_H

#include "task.h"
#include "transform.h"
#include "communication.h"

#define READ 0
#define WRITE 1

void input_loop(int pipe);

/*
task #3: proc-file 0 /home/user/samples/file-c file-c-output nop bcompress
task #5: proc-file 1 samples/file-a file-a-output bcompress nop gcompress encrypt nop
task #8: proc-file 1 file-b-output path/to/dir/new-file-b decrypt gdecompress
transf nop: 3/3 (running/max)
transf bcompress: 2/4 (running/max)
transf bdecompress: 1/4 (running/max)
transf gcompress: 1/2 (running/max)
transf gdecompress: 1/2 (running/max)
transf encrypt: 1/2 (running/max)
transf decrypt: 1/2 (running/max)
*/
void status(char *pipe_in);

/*
pending
processing
concluded (bytes-input: 2048, bytes-output: 1024)
*/
void proc_file(Comms msg, Cmd cmd);

void free_transf();

void fork_off(Task task);

void exec_list(Task task);

void apply_transf(Task task);

#endif