#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/parser.h"
#include "../include/scheduler.h"

int main(int argc, char *argv[]){
    char *input_file = NULL;
    int verbose = 0;

    // Parseo básico de argumentos CLI
    for(int i = 1; i < argc; i++){
        if(strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "-verbose") == 0){
            verbose = 1;
        }else if(strcmp(argv[i], "--input") == 0 && i + 1 < argc){
            input_file = argv[++i];
        }
    }

    if(!input_file){
        fprintf(stderr, "Uso: %s [-v] --input <input_file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int task_count = 0;
    TCB **tasks = parser_parse_file(input_file, &task_count);

    Scheduler sched;
    scheduler_init(&sched, tasks, task_count, verbose);
    scheduler_run(&sched);
    scheduler_print_metrics(&sched); // Imprimir metricas
    scheduler_destroy(&sched);

    parser_free_tasks(tasks, task_count);
    return EXIT_SUCCESS;
}