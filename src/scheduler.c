#include <stdio.h>
#include <stdlib.h>
#include "../include/scheduler.h"

static void* master_thread_func(void *arg) {
    Scheduler *sched = (Scheduler*) arg;
    // TODO: Hilo Maestro - Evaluar arribos por tic Tg[cite: 3]
    return NULL;
}

static void* worker_thread_func(void *arg) {
    Scheduler *sched = (Scheduler*) arg;
    // TODO: Hilo CPU - Expiración, Expropiación, Despacho y Cómputo[cite: 2, 3]
    return NULL;
}

void scheduler_init(Scheduler *sched, TCB **tasks, int count, int verbose) {
    queue_init(&sched->queue0);
    queue_init(&sched->queue1);
    queue_init(&sched->queue2);

    sched->allTasks = tasks;
    sched->totalTasks = count;
    sched->runningTask = NULL;
    sched->globalTick = 0;
    sched->verbose = verbose;

    pthread_mutex_init(&sched->lock, NULL);
    pthread_barrier_init(&sched->barrier, NULL, 2); // 2 hilos: Maestro y CPU[cite: 3]
}

void scheduler_run(Scheduler *sched) {
    pthread_t master_thread, worker_thread;

    // Crear los dos hilos requeridos[cite: 3]
    pthread_create(&master_thread, NULL, master_thread_func, sched);
    pthread_create(&worker_thread, NULL, worker_thread_func, sched);

    // Esperar a que terminen la simulación
    pthread_join(master_thread, NULL);
    pthread_join(worker_thread, NULL);

    printf("Simulación finalizada correctamente.\n");
}

void scheduler_destroy(Scheduler *sched) {
    queue_clear(&sched->queue0);
    queue_clear(&sched->queue1);
    queue_clear(&sched->queue2);

    pthread_mutex_destroy(&sched->lock);
    pthread_barrier_destroy(&sched->barrier);
}