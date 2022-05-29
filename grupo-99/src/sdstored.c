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

sig_atomic_t isRunning = 1;

void sigint_handler(int signum) {
    isRunning = 0;
}

// ./sdstored etc/sdstored.conf bin/sdstore-transformations
int main(int argc, char* argv[]) {
    exit_if((argc != 3), "./sdstored config-filename transformations-folder\n");

    signal(SIGINT, sigint_handler);

    transf_max = load_transforms(argv[1], transf);
    path = strdup(argv[2]);

    exit_if((mkfifo(AUTH_PIPE, 0666) != 0), "main fifo creation");
    int auth = open(AUTH_PIPE, O_RDONLY | O_NONBLOCK);

    while (isRunning) {
        free_transf();
        input_loop(auth);
    }

    close(auth);
    unlink(AUTH_PIPE);

    while(tasks != NULL)
        free_transf();

    return 0;
}

void input_loop(int pipe) {
    Comms msg;
    Cmd cmd;

    if (read(pipe, &msg, sizeof(Comms)) > 0) {
        int input = open(msg.pipe_c2s, O_RDONLY);
        exit_if((input == -1), "erro pipe c2s");
        read(input, &cmd, sizeof(Cmd));
        close(input);

        if (!strcmp(cmd.argv[0], "status")) {
            status(msg.pipe_s2c);
        } else if (!strcmp(cmd.argv[0], "proc-file")) {
            proc_file(msg, cmd);
        }
    }
}

void proc_file(Comms msg, Cmd cmd) {
    int pipe = open(msg.pipe_s2c, O_WRONLY);

    Response res = new_res("Pending\n");
    write(pipe, &res, sizeof(Response));
    Task task = new_task(msg, cmd);

    int availability = check_available(task, transf, transf_max);
    if (availability == -2) {
        res = new_res("One or more transformations does not exist\n");
        write(pipe, &res, sizeof(Response));
        res = new_res("");
        write(pipe, &res, sizeof(Response));
    }else if (availability == -1) {
        res = new_res("One or more transformations is over the limit\n");
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

void status(char *pipe_in) {
    int pipe = open(pipe_in, O_WRONLY);

    Response res = new_res("");

    for (Task ptr = tasks; ptr != NULL; ptr = ptr->prox) {
        res.sent = sprintf(
            res.line,
            "task #%2d: proc-file %d %s %s\n",
            ptr->count, ptr->priority, ptr->input, ptr->output
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

void free_transf() {
    pid_t pid;

    while ((pid = waitpid(-1, NULL, WNOHANG)) > 0) {
        for (Task ptr = tasks; ptr != NULL; ptr = ptr->prox) {
            if (pid == ptr->task_pid) {
                decrement_transf(ptr, transf, transf_max);
                tasks = rem_task(tasks, ptr);
                break;
            }
        }

        for (Task ptr = tasks; ptr != NULL; ptr = ptr->prox) {
            if (check_available(ptr, transf, transf_max) && !ptr->processing)
                fork_off(ptr);
        }
    }
}

void fork_off(Task task) {
    increment_transf(task, transf, transf_max);
    task->processing = 1;

    pid_t pid = fork();

    exit_if((pid == -1), "Fork_off error");

    if (pid == 0) {
        apply_transf(task);
    } else {
        task->task_pid = pid;
    }
}

void apply_transf(Task task) {
    // para testar concorrência
    sleep(TESTING);

    int pipe = open(task->client.pipe_s2c, O_WRONLY);

    Response res = new_res("Processing\n");
    write(pipe, &res, sizeof(Response));

    exec_list(task);

    int size_in = size_of_file(task->input);
    int size_out = size_of_file(task->output);

    res.sent = sprintf(res.line, "Concluded (bytes-input: %d, bytes-output: %d)\n", size_in, size_out);
    write(pipe, &res, sizeof(Response));
    res = new_res("");
    write(pipe, &res, sizeof(Response));

    _exit(EXIT_SUCCESS);
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