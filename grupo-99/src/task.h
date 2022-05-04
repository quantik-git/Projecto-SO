#ifndef TASK_H
#define TASK_H

#include <stdlib.h>
#include <stdio.h>

#include "communication.h"

typedef struct task {
    Comms client;
    int count;
    int task_pid;
    int priority;
    int transform_count;
    int processing;
    char input[64];
    char output[64];
    char transforms[64][64];
    struct task *prox;
} *Task;

// Cria a task
Task new_task(Comms client, Cmd cmd);

// Insere de forma ordenada por prioridade
Task add_task(Task tasks, Task new);

Task rem_task(Task tasks, Task task);

#endif