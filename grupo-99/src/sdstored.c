#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "utils.h"

char *path;

typedef struct task {
    Auth msg; // não sei se preciso mesmo disto
    int priority;
    int transform_count;
    int processing;
    char input[64];
    char output[64];
    char transforms[64][64];
    struct task *prox;
} *Task;

Task tasks = NULL;

// Cria a task e insere de forma ordenada por prioridade
Task task_new(Auth msg, Cmd cmd) {
    Task new = malloc(sizeof(struct task));
    new->msg = msg;
    new->priority = atoi(cmd.argv[1]);
    new->transform_count = cmd.argc-4;
    new->processing = 0;
    strcpy(new->input, cmd.argv[2]);
    strcpy(new->output, cmd.argv[3]);
    for (int i = 4; i < cmd.argc; i++) {
        strcpy(new->transforms[i-4], cmd.argv[i]);
    }
    new->prox = NULL;

    return new;
}

void add_task(Task new) {
    if (tasks == NULL) {
        tasks = new;
    } else {
        Task ptr;
        for (ptr = tasks; ptr->prox != NULL; ptr = ptr->prox) {
            if (new->priority > (ptr->prox)->priority)
                break;
        }

        new->prox = ptr->prox;
        ptr->prox = new;
    }
}

void rem_task(Task task) {
    if (tasks == NULL) return;

    Task ant, ptr;
    for (ant = NULL, ptr = tasks; ptr != NULL; ant = ptr, ptr = ptr->prox) {
        if (!strcmp(task->msg.pid, ptr->msg.pid))
            break;
    }

    if (ant == NULL)
        tasks = ptr->prox;
    else
        ant->prox = ptr->prox;

    free(task);
}

//nop 2
typedef struct transform {
    int atual;
    int max;
    char *nome;
} Transform;

Transform transf[128];
int transf_max = 0;

void load_transforms(char *file) {
    int src = open(file, O_RDONLY);
    exit_if((src == -1), "erro na abertura do ficheiro de input");

    char buff[128];
    while (read_segment(src, buff, sizeof(buff), '\n') > 0) {
        transf[transf_max].nome = strdup(strtok(buff, " "));
        transf[transf_max].max = atoi(strtok(NULL, " "));
        transf[transf_max].atual = 0;
        transf_max++;
    }
}

int check_available(Task t) {
    for (int i = 0; i < transf_max; i++) {
        int count = 0;
        for (int j = 0; j < t->transform_count; j++) {
            if (!strcmp(transf[i].nome, t->transforms[j])) {
                count++;
            }
        }
        
        if (count > transf[i].max) {
            return -1;
        } else if ((count + transf[i].atual) > transf[i].max) {
            return 0;
        }
    }

    return 1;
}

void increment_transf(Task task) {
    for (int i = 0; i < transf_max; i++) {
        for (int j = 0; j < task->transform_count; j++) {
            if (!strcmp(transf[i].nome, task->transforms[j])) {
                transf[i].atual++;
            }
        }
    }
}

void decrement_transf(Task task) {
    for (int i = 0; i < transf_max; i++) {
        for (int j = 0; j < task->transform_count; j++) {
            if (!strcmp(transf[i].nome, task->transforms[j])) {
                transf[i].atual--;
            }
        }
        printf("%s - %d\n", transf[i].nome, transf[i].atual);
    }
}

void status(char *pipe_in);
void proc_file(Auth msg, Cmd cmd);
void fork_off(Task task);
void apply_transf(Task task);
void exec_list(Task task);
void free_transf();

sig_atomic_t term_count = 0;

void fork_off(Task task) {
    increment_transf(task);
    task->processing = 1;
    pid_t pid = fork();

    exit_if((pid == -1), "Fork_off error");

    if (pid == 0) {
        // run shit
        apply_transf(task);
    }
}

void apply_transf(Task task) {
    sleep(15); // para testar concorrencia
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
    write(terminated, &(task->msg), sizeof(Auth));
    close(terminated);

    kill(pai, SIGUSR1);
    printf("sent pai\n");
    kill(getpid(), SIGKILL);
    printf("sent himself\n");
}

void sigint_handler(int signum) {
    unlink(AUTH_PIPE);
    exit(0);
}

void sigusr1_handler(int signum) {
    term_count++;
    printf("sighdl %d\n", term_count);
    free_transf();
}

void free_transf() {
    //if (!term_count) return;
    Auth msg;
    Task ptr;

    printf("cleaning up\n");

    int pipe = open(TERM_PIPE, O_RDONLY);
    while (read(pipe, &msg, sizeof(Auth)) > 0) {
        for (ptr = tasks; ptr != NULL; ptr = ptr->prox) {
            if (!strcmp(msg.pid, ptr->msg.pid))
                break;
        }
        decrement_transf(ptr);
        rem_task(ptr);
        term_count--;
    }
    close(pipe);

    for (ptr = tasks; ptr != NULL; ptr = ptr->prox) {
        if (check_available(ptr) && !ptr->processing)
            fork_off(ptr);
    }
}

#define READ 0
#define WRITE 1

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

// ./sdstored etc/sdstored.conf bin/sdstore-transformations
int main(int argc, char* argv[]) {
    //exit_if((argc != 3), "./sdstored config-filename transformations-folder\n");
    signal(SIGUSR1, sigusr1_handler);
    signal(SIGINT, sigint_handler);
    load_transforms(argv[1]);
    path = strdup(argv[2]);

    exit_if(mkfifo(AUTH_PIPE, 0666) == -1, "main fifo creation");
    //exit_if(mkfifo(TERM_PIPE, 0666) == -1, "term fifo creation");

    printf("%d\n", getpid());

    while (1) {
        Auth msg;
        Cmd cmd;

        int auth = open(AUTH_PIPE, O_RDONLY);
        read(auth, &msg, sizeof(Auth));
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
void proc_file(Auth msg, Cmd cmd) {
    int pipe = open(msg.pipe_s2c, O_WRONLY);

    Response res = new_res("Pending\n");
    write(pipe, &res, sizeof(Response));
    Task task = task_new(msg, cmd);

    int availability = check_available(task);
    if (availability == -1) {
        res = new_res("Too many transformations required\n");
        write(pipe, &res, sizeof(Response));
        res = new_res("");
        write(pipe, &res, sizeof(Response));
    } else if (availability == 0) {
        // tem de ficar em espera
        add_task(task);
    } else {
        // pode começar a transformar
        add_task(task);
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