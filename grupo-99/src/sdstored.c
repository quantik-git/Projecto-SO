#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>

#include "sdstored.h"


char *path;
Task tasks = NULL;
Transform transf[128];
int transf_max = 0;

sig_atomic_t term_count = 0;

void sigint_handler(int signum) {
    unlink(AUTH_PIPE);
    exit(0);
}

void sigusr1_handler(int signum) {
    term_count++;
    printf("sighdl %d\n", term_count);
    free_transf();
}

// ./sdstored etc/sdstored.conf bin/sdstore-transformations
int main(int argc, char* argv[]) {
    //exit_if((argc != 3), "./sdstored config-filename transformations-folder\n");
    signal(SIGUSR1, sigusr1_handler);
    signal(SIGINT, sigint_handler);
    transf_max = load_transforms(argv[1], transf);
    path = strdup(argv[2]);

    exit_if(mkfifo(AUTH_PIPE, 0666) == -1, "main fifo creation");
    //exit_if(mkfifo(TERM_PIPE, 0666) == -1, "term fifo creation");

    printf("%d\n", getpid());

    while (1) {
        Comms msg;
        Cmd cmd;

        int auth = open(AUTH_PIPE, O_RDONLY);
        read(auth, &msg, sizeof(Comms));
        close(auth);

        int in = open(msg.pipe_c2s, O_RDONLY);
        exit_if((in == -1), "erro pipe c2s");
        printf("open %s\n", msg.pipe_c2s);

        read(in, &cmd, sizeof(Cmd));
        printf("received %s\n", cmd.argv[0]);
        close(in);

        if (!strcmp(cmd.argv[0], "status")) {
            status(msg.pipe_s2c);
        } else if (!strcmp(cmd.argv[0], "proc-file")) {
            proc_file(msg, cmd);
        }

        //free_transf();
    }

    return 0;
}

/*
pending
processing
concluded (bytes-input: 2048, bytes-output: 1024)
*/
void proc_file(Comms msg, Cmd cmd) {
    int pipe = open(msg.pipe_s2c, O_WRONLY);

    Response res = new_res("Pending\n");
    write(pipe, &res, sizeof(Response));
    Task task = new_task(msg, cmd);

    int availability = check_available(task, transf, transf_max);
    if (availability == -1) {
        res = new_res("Too many transformations required\n");
        write(pipe, &res, sizeof(Response));
        res = new_res("");
        write(pipe, &res, sizeof(Response));
    } else if (availability == 0) {
        // tem de ficar em espera
        tasks = add_task(tasks, task);
    } else {
        // pode começar a transformar
        tasks = add_task(tasks, task);
        fork_off(task);
    }

    close(pipe);
}

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
void status(char *pipe_in) {
    int pipe = open(pipe_in, O_WRONLY);

    printf("funciona\n");
    Response res = new_res("funciona\n");
    write(pipe, &res, sizeof(Response));

    for (Task ptr = tasks; ptr != NULL; ptr = ptr->prox) {
        res.sent = sprintf(
            res.line,
            "task #%2d: proc-file %d %s %s\n",
            0, ptr->priority, ptr->input, ptr->output
        );
        write(pipe, &res, sizeof(Response));
    }

    for (int i = 0; i < transf_max; i++) {
        res.sent = sprintf(
            res.line,
            "transf %s: %d/%d (running/max)\n",
            transf[i].nome, transf[i].atual, transf[i].max
        );
        write(pipe, &res, sizeof(Response));
    }

    res = new_res("");
    write(pipe, &res, sizeof(Response));

    close(pipe);
}
void fork_off(Task task) {

    increment_transf(task, transf, transf_max);
    task->processing = 1;
    pid_t pid = fork();

    exit_if((pid == -1), "Fork_off error");

    if (pid == 0) {
        // run shit
        apply_transf(task);
    }
}

void apply_transf(Task task) {
    sleep(5); // para testar concorrencia
    int pipe = open(task->msg.pipe_s2c, O_WRONLY);

    pid_t pai = getppid();

    printf("%d\n", pai);

    Response res = new_res("Processing\n");
    write(pipe, &res, sizeof(Response));

    exec_list(task);

    int n1 = 0, n2 = 0;
    res.sent = sprintf(res.line, "Concluded (bytes-input: %d, bytes-output: %d)\n", n1, n2);
    write(pipe, &res, sizeof(Response));
    res = new_res("");
    write(pipe, &res, sizeof(Response));

    int terminated = open(TERM_PIPE, O_WRONLY | O_CREAT | O_TRUNC, 0777);
    write(terminated, &(task->msg), sizeof(Comms));
    close(terminated);

    kill(pai, SIGUSR1);
    printf("sent pai\n");
    kill(getpid(), SIGKILL);
    printf("sent himself\n");
}

void free_transf() {
    //if (!term_count) return;
    Comms msg;
    Task ptr;

    printf("cleaning up\n");

    int pipe = open(TERM_PIPE, O_RDONLY);
    while (read(pipe, &msg, sizeof(Comms)) > 0) {
        for (ptr = tasks; ptr != NULL; ptr = ptr->prox) {
            if (!strcmp(msg.pid, ptr->msg.pid))
                break;
        }
        decrement_transf(ptr, transf, transf_max);
        tasks = rem_task(tasks, ptr);
        term_count--;
    }
    close(pipe);

    for (ptr = tasks; ptr != NULL; ptr = ptr->prox) {
        if (check_available(ptr, transf, transf_max) && !ptr->processing)
            fork_off(ptr);
    }
}

void exec_list(Task task) {
    int pipe_fds[2][2], ptr;
    int input = open(task->input, O_RDONLY);
    int output = open(task->output, O_WRONLY | O_CREAT | O_TRUNC, 0777);

    for(int i = 0; i < task->transform_count; i++) {
        ptr = i % 2;

        char transform[128];
        sprintf(transform, "%s/%s", path, task->transforms[i]);

        if (i == 0) {
            // first
            pipe(pipe_fds[ptr]);

            if (fork() == 0) {
                dup2(input, STDIN_FILENO);
                close(input);
                close(pipe_fds[ptr][READ]);
                if (task->transform_count == 1) {
                    dup2(output, STDOUT_FILENO);
                } else {
                    dup2(pipe_fds[ptr][WRITE], STDOUT_FILENO);
                }
                close(pipe_fds[ptr][WRITE]);

                execlp(transform, transform, NULL);
                _exit(EXIT_FAILURE);
            }
            close(pipe_fds[ptr][WRITE]);
            wait(NULL);
        } else if (i+1 == task->transform_count) {
            // last
            if (fork() == 0) {
                dup2(output, STDOUT_FILENO);
                close(output);
                dup2(pipe_fds[(ptr+1) % 2][READ], STDIN_FILENO);
                close(pipe_fds[(ptr+1) % 2][READ]);

                execlp(transform, transform, NULL);
                _exit(EXIT_FAILURE);
            }
            close(pipe_fds[(ptr+1) % 2][READ]);
            wait(NULL);
        } else {
            // middle
            pipe(pipe_fds[ptr]);

            if (fork() == 0) {
                dup2(pipe_fds[(ptr+1) % 2][READ], STDIN_FILENO);
                close(pipe_fds[(ptr+1) % 2][READ]);
                dup2(pipe_fds[ptr][WRITE], STDOUT_FILENO);
                close(pipe_fds[ptr][WRITE]);

                execlp(transform, transform, NULL);
                _exit(EXIT_FAILURE);
            }
            close(pipe_fds[(ptr+1) % 2][READ]);
            close(pipe_fds[ptr][WRITE]);
            wait(NULL);
        }
    }
}