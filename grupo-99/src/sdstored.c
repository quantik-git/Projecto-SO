#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include "utils.h"

void exit_if(int cond, char* message) {
    if (cond) {
        perror(message);
        exit(EXIT_FAILURE);
    }
}

//nop 2
typedef struct transform {
    int atual;
    int maximo;
    char *nome_transform;
    struct transform *prox;
} *Transform;

Transform load_transforms(char *file) {
    int src = open(argv[1], O_RDONLY);
	exit_if((src == -1), "erro na abertura do ficheiro de input");
}

// ./sdstored etc/sdstored.conf bin/sdstore-transformations
int main(int argc, char* argv[]) {
    char* path;

    exit_if((argc != 3), "./sdstored config-filename transformations-folder\n");

    filtros = lerConfig(argv[1]);
    path = argv[2];

    exit_if(mkfifo("tmp/main", 0666), "main fifo creation");

    while (1) {
        int pipe = open("tmp/main", O_RDONLY);
        char pid[MAXBUFFER];
        int res = 0;

        while (read(pipe, pid+res,1) > 0) {
            res++;
        }
        pid[res++] = '\0';


        char pid_ler_cliente[strlen(pid)+5];
        strcpy(pid_ler_cliente, "tmp/w");
        strcpy(pid_ler_cliente+5,pid);
        res = 0;
        
        char info_cliente[MAXBUFFER];
        int pipe_ler_cliente = open(pid_ler_cliente, O_RDONLY);

        while (read(pipe_ler_cliente, info_cliente+res,1) > 0){
            res++;
        }
        info_cliente[res] = '\0';

        if (info_cliente[0] == 's') status(pid);
        else transform(pid, info_cliente);
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
    STATUS_CODE status;// 0 - a espera; 1 - a processar
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