#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <fcntl.h>

#include "task.h"
#include "utils.h"

//nop 2
typedef struct transform {
    int atual;
    int max;
    char *nome;
} Transform;

int load_transforms(char *file, Transform transf[]);

/**
 * -1 impossible
 *  0 not available
 *  1 available
 */
int check_available(Task t, Transform transf[], int size);

void increment_transf(Task task, Transform transf[], int size);

void decrement_transf(Task task, Transform transf[], int size);

#endif