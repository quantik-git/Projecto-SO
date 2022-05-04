#include "task.h"


Task new_task(Comms client, Cmd cmd) {
    Task new = malloc(sizeof(struct task));
    new->client = client;
    new->count = 0;
    new->task_pid = 0;
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

Task add_task(Task tasks, Task new) {
    if (tasks == NULL) {
        tasks = new;
    } else {
        Task ptr, ant = NULL;
        for (ptr = tasks; ptr != NULL; ant = ptr, ptr = ptr->prox) {
            if (new->priority > ptr->priority)
                break;
        }

        if (ant == NULL) {
            new->prox = ptr;
            tasks = new;
        } else {
            new->prox = ant->prox;
            ant->prox = new;
            
        }
    }

    // fprintf(stderr, "here\n");
    // for (Task ptr = tasks; ptr != NULL; ptr = ptr->prox)
    //     fprintf(stderr, "%d %d ->", ptr->task_pid, ptr->priority);
    
    // fprintf(stderr, "\n%d\n", tasks->priority);

    return tasks;
}

Task rem_task(Task tasks, Task task) {
    if (tasks == NULL) return NULL;

    Task ant, ptr;
    for (ant = NULL, ptr = tasks; ptr != NULL; ant = ptr, ptr = ptr->prox) {
        if (!strcmp(task->client.pid, ptr->client.pid))
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