#include "transform.h"


int load_transforms(char *file, Transform transf[]) {
    int src = open(file, O_RDONLY);
    exit_if((src == -1), "erro na abertura do ficheiro de config");

    char buff[128];
    int i;
    for (i = 0; read_segment(src, buff, sizeof(buff), '\n') > 0; i++) {
        transf[i].nome = strdup(strtok(buff, " "));
        transf[i].max = atoi(strtok(NULL, " "));
        transf[i].atual = 0;
    }

    return i;
}

/**
 * -1 impossible
 *  0 not available
 *  1 available
 */
int check_available(Task t, Transform transf[], int size) {
    for (int i = 0; i < size; i++) {
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

void increment_transf(Task task, Transform transf[], int size) {
    for (int i = 0; i < size; i++)
        for (int j = 0; j < task->transform_count; j++)
            if (!strcmp(transf[i].nome, task->transforms[j]))
                transf[i].atual++;
}

void decrement_transf(Task task, Transform transf[], int size) {
    for (int i = 0; i < size; i++)
        for (int j = 0; j < task->transform_count; j++)
            if (!strcmp(transf[i].nome, task->transforms[j]))
                transf[i].atual--;
}