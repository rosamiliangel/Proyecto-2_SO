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

    // De manera temporal, probamos el parser
    if(!tasks || task_count == 0){
        printf("No se cargaron centinelas del archivo.\n");
        return EXIT_FAILURE;
    }

    printf("\n=== CENTINELAS CARGADOS Y ORDENADOS (%d en total) ===\n", task_count);
    printf("%-10s %-10s %-12s %-12s\n", "Llegada", "ID", "Prioridad", "Rafaga Total");
    printf("--------------------------------------------------\n");

    for(int i = 0; i < task_count; i++){
        printf("%-10d %-10d %-12d %-12d\n", tasks[i]->arrivalTime, tasks[i]->id, tasks[i]->priority, tasks[i]->totalBurst);
    }
    printf("================================------------------\n\n");

    Scheduler sched;
    scheduler_init(&sched, tasks, task_count, verbose);
    scheduler_run(&sched);
    scheduler_destroy(&sched);

    parser_free_tasks(tasks, task_count);
    return EXIT_SUCCESS;
}