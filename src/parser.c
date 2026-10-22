#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "../include/parser.h"

#define MAX_LINE_LENGTH 500

// Función para ordenar los centinelas por Tiempo de Llegada
static int comparar_tasks_por_llegada(const void *a, const void *b){
    TCB *taskA = *(TCB**)a;
    TCB *taskB = *(TCB**)b;
    return taskA->arrivalTime - taskB->arrivalTime;
}

TCB** parser_parse_file(const char *filename, int *out_count){
    FILE *file = fopen(filename, "r");
    if(!file){
        perror("Error al abrir el archivo de entrada");
        *out_count = 0;
        return NULL;
    }

    int capacidad = 10;
    int contador = 0;
    TCB **tasks = (TCB**) malloc(capacidad * sizeof(TCB*));
    if(!tasks){
        perror("Error de asignación de memoria para tareas");
        fclose(file);
        *out_count = 0;
        return NULL;
    }

    char line[MAX_LINE_LENGTH];

    while(fgets(line, sizeof(line), file)){
        // Ignorar espacios en blanco del inicio
        char *ptr = line;
        while(isspace((unsigned char)*ptr)) ptr++;
        // Ignorar líneas vacías o comentarios que inician con el #
        if(*ptr == '#' || *ptr == '\0' || *ptr == '\n') continue;

        int arrivalTime, id, priority, totalBurst;

        // Extraer los 4 campos esperados
        if(sscanf(ptr, "%d %d %d %d", &arrivalTime, &id, &priority, &totalBurst) == 4){
            TCB *task = tcb_create(id, priority, arrivalTime, totalBurst);
            if(!task){
                fprintf(stderr, "Error creando TCB para centinela ID: %d\n", id);
                continue;
            }
            // Redimensionar el arreglo si sobrepasa la capacidad actual
            if(contador >= capacidad){
                capacidad *= 2;
                TCB **temp = (TCB**) realloc(tasks, capacidad * sizeof(TCB*));
                if (!temp) {
                    perror("Error reasignando memoria para el arreglo de tareas");
                    break;
                }
                tasks = temp;
            }
            tasks[contador++] = task;
        }
    }

    fclose(file);
    // Ordenar las tareas por tiempo de llegada de forma ascendente
    if(contador > 0){
        qsort(tasks, contador, sizeof(TCB*), comparar_tasks_por_llegada);
    }

    *out_count = contador;
    return tasks;
}

void parser_free_tasks(TCB **tasks, int count) {
    if(!tasks) return;
    for(int i = 0; i < count; i++){
        tcb_destroy(tasks[i]);
    }
    free(tasks);
}