#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "utils.h"

//nop 2
typedef struct transform {
    int atual;
    int maximo;
    char *nome_transform;
    struct transform *prox;
} *Transform;

Transform load_transforms(char *file) {
    int src = open(file, O_RDONLY);
    exit_if((src == -1), "erro na abertura do ficheiro de input");

    Transform t = NULL;

    return t;
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
void status(char *pid) {
    char buffer[256];
    ssize_t n_read;

    char pipe_in[30];
    sprintf(pipe_in, "in_%s", pid);

    int pipe = open(pipe_in, O_WRONLY);

    printf("\nfunciona\n");
    write(pipe, "funciona\n", sizeof("funciona\n"));

    // while task
    // n_read = sprintf(buffer, "task #%d: proc-file %d %s ", ); //aqui não tenho a certeza se pode ficar assim
    // write(pipe, buffer, n_read);

    // while (tranf)
    // n_read = sprintf(buffer, "transf %s: %d/%d (running/max)", );
    // write(pipe, buffer, n_read);

    close(pipe);
}

void proc_file(char *pid) {
    char pipe_in[30];
    sprintf(pipe_in, "in_%s", pid);

    int pipe = open(pipe_in, O_WRONLY);

    write(pipe, "Pending\n", sizeof("Pending\n"));
    // ver o status
    // quando tiver processos livres suficientes

    write(pipe, "Processing\n", sizeof("Processing\n"));

    // começa a processar
    // quando termina

    char buffer[256];
    ssize_t n_read;
    int n1 = 0;
    int n2 = 0;
    n_read = sprintf(buffer, "Concluded (bytes-input: %d, bytes-output: %d)", n1, n2);
    write(pipe, buffer, n_read);
    close(pipe);
}

// ./sdstored etc/sdstored.conf bin/sdstore-transformations
int main(int argc, char* argv[]) {
    /*char* path;

    exit_if((argc != 3), "./sdstored config-filename transformations-folder\n");

    Transform transforms = load_transforms(argv[1]);
    path = strdup(argv[2]);*/

    exit_if(mkfifo(AUTH_PIPE, 0666) == -1, "main fifo creation");

    while (1) {
        int pipe = open(AUTH_PIPE, O_RDONLY);
        char buffer[128];
        char pid[16];
        ssize_t n_read;

        while(n_read = read(pipe, buffer, 10)) {
            write(STDOUT_FILENO, buffer, n_read);
            strcpy(pid, buffer);
        }

        char pipe_out[30];
        sprintf(pipe_out, "out_%s", pid);

        printf("\t%s\n", pipe_out);

        int out = open(pipe_out, O_RDONLY);

        while(n_read = read(out, buffer, 32)) {
            write(STDOUT_FILENO, buffer, n_read);
        }

        if (!strcmp(buffer, "status")) {
            status(pid);
        } else if (!strcmp(argv[1], "proc-file")) {
            proc_file(pid);
        }
    }

    return 0;
}



typedef enum status {
    PENDING = 0,
    PROCESSING = 1,
} STATUS_CODE;

static const char * const STATUS_TEXT[] = {
    [PENDING] = "Pending",
    [PROCESSING] = "Processing"
};

typedef struct task {
    STATUS_CODE status; // 0 - a espera; 1 - a processar
    char *input_file;
    char *output_file;
    int prioridade; // talvez fazer um enum para isto

    char *comando;
    char **ordem_filtros;
    struct quantidade_filtro *nomes_filtros;
    int numero;
    int pid;

    struct task *prox;
} *Task;