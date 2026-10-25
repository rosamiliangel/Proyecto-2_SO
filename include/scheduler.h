#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <pthread.h>
#include "queue.h"

typedef struct {
    Queue queue0;            // Alta prioridad (RR Q=2)
    Queue queue1;            // Media prioridad (RR Q=4)
    Queue queue2;            // Baja prioridad (FCFS)
    
    TCB **allTasks;          // Arreglo completo de centinelas
    int totalTasks;          // Cantidad total de tareas
    
    TCB *runningTask;        // Centinela activo en CPU
    int globalTick;          // Reloj global
    int verbose;             // Indica si se debe imprimir la salida de depuracion
    
    pthread_mutex_t lock;
    pthread_barrier_t barrier;
    pthread_barrier_t barrier_end;
} Scheduler;

void scheduler_init(Scheduler *sched, TCB **tasks, int count, int verbose);
void scheduler_run(Scheduler *sched);
void scheduler_print_metrics(Scheduler *sched);
void scheduler_destroy(Scheduler *sched);

#endif