#include "task.h"

// Cria a task
Task new_task(Comms msg, Cmd cmd) {
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

// Insere de forma ordenada por prioridade
Task add_task(Task tasks, Task new) {
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

    return tasks;
}

Task rem_task(Task tasks, Task task) {
    if (tasks == NULL) return NULL;

    Task ant, ptr;
    for (ant = NULL, ptr = tasks; ptr != NULL; ant = ptr, ptr = ptr->prox) {
        if (!strcmp(task->msg.pid, ptr->msg.pid))
            break;
    }

    if (ant == NULL) {
        tasks = ptr->prox;
    } else {
        ant->prox = ptr->prox;
    }

    free(task);
    return tasks;
}