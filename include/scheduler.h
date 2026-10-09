#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <pthread.h>
#include "queue.h"

typedef struct {
    Queue queue0;            // Alta prioridad (RR Q=2)[cite: 1]
    Queue queue1;            // Media prioridad (RR Q=4)[cite: 1]
    Queue queue2;            // Baja prioridad (FCFS)[cite: 1]
    
    TCB **allTasks;          // Arreglo completo de centinelas
    int totalTasks;          // Cantidad total
    
    TCB *runningTask;        // Centinela activo en CPU
    int globalTick;          // Reloj discreto Tg[cite: 1]
    int verbose;             // Muestra trazas en cada tic[cite: 4]
    
    pthread_mutex_t lock;
    pthread_barrier_t barrier;
} Scheduler;

void scheduler_init(Scheduler *sched, TCB **tasks, int count, int verbose);
void scheduler_run(Scheduler *sched);
void scheduler_destroy(Scheduler *sched);

#endif