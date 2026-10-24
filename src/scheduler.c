#include <stdio.h>
#include <stdlib.h>
#include "../include/scheduler.h"

static void* master_thread_func(void *arg) {
    Scheduler *sched = (Scheduler*) arg;

    pthread_mutex_lock(&sched->lock); // Protección de la sección crítica
    for(int i = 0; i < sched->totalTasks; i++){
        TCB *task = sched->allTasks[i];
        // Si el centinela llega en el tic global actual
        if(task->arrivalTime == sched->globalTick && task->state == STATE_NEW){
            task->state = STATE_READY;
            // Enrutar a la cola según su prioridad estricta
            if(task->priority == 0){
                queue_enqueue(&sched->queue0, task);
                if(sched->verbose){
                    printf("[T-%04d] ARRIVAL: Sentinel ID: %d [Type: Command]\n", sched->globalTick, task->id);
                }
            }else if(task->priority == 1){
                queue_enqueue(&sched->queue1, task);
                if(sched->verbose){
                    printf("[T-%04d] ARRIVAL: Sentinel ID: %d [Type: Driller]\n", sched->globalTick, task->id);
                }
            }else if (task->priority == 2){
                queue_enqueue(&sched->queue2, task);
                if(sched->verbose){
                    printf("[T-%04d] ARRIVAL: Sentinel ID: %d [Type: Heavy]\n", sched->globalTick, task->id);
                }
            }
        }
    }
    pthread_mutex_unlock(&sched->lock); // Libere la sección crítica

    return NULL;
}

static void* worker_thread_func(void *arg) {
    Scheduler *sched = (Scheduler*) arg;
    // TODO: Hilo CPU - Expiración, Expropiación, Despacho y Cómputo
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
    pthread_barrier_init(&sched->barrier, NULL, 2); // 2 hilos: Maestro y CPU
}

/*
void scheduler_run_oring(Scheduler *sched) {
    pthread_t master_thread, worker_thread;

    // Crear los dos hilos requeridos[cite: 3]
    pthread_create(&master_thread, NULL, master_thread_func, sched);
    pthread_create(&worker_thread, NULL, worker_thread_func, sched);

    // Esperar a que terminen la simulación
    pthread_join(master_thread, NULL);
    pthread_join(worker_thread, NULL);

    printf("Simulación finalizada correctamente.\n");
}
*/

void scheduler_run(Scheduler *sched) {
    int TareasCompletadas = 0;
    
    // LLegada de procesos y encolamiento según prioridad
    while (TareasCompletadas < sched->totalTasks) {
        for (int i = 0; i < sched->totalTasks; i++) {
            TCB *task = sched->allTasks[i];
            if (task->arrivalTime == sched->globalTick && task->state == STATE_NEW) {
                task->state = STATE_READY;

                if (task->priority == 0) {queue_enqueue(&sched->queue0,task);}
                else if (task->priority == 1) {queue_enqueue(&sched->queue1,task);}
                else if (task->priority == 2) {queue_enqueue(&sched->queue2,task);}

                if (sched->verbose) {printf("[T-%4d] ARRIVAL: Sentinel ID: %d [Type: %10s]\n", sched->globalTick, task->id, task->priority, TiposPrioridad[task->priority]);}
            }
        }

        // Ejecución de procesos
        if (sched->runningTask !=NULL) {
            TCB *task = sched->runningTask;
            task->usedQuantum++;
            task->remainingBurst--;

            int LimiteQuantum = (task->priority == 0) ? 2 : ((task->priority == 1) ? 4: 0);
            int Apropiado = 0;
            
            if (task-> priority > 0 && !queue_is_empty(&sched->queue0)) {Apropiado = 1;}
            if (task-> priority == 2 && !queue_is_empty(&sched->queue1)) {Apropiado = 1;}

            if (task->remainingBurst == 0) {
                task->state = STATE_FINISHED;
                task->finishTime = sched->globalTick + 1;
                task->returnTime = task->finishTime - task->arrivalTime;
                sched->runningTask = NULL;
                TareasCompletadas++;
                if (sched->verbose) {printf("[T-%4d] COMPLETED: Sentinel ID: %d\n", sched->globalTick + 1, task->id);}
            }
            else if (LimiteQuantum >0 && task->usedQuantum == LimiteQuantum) {
                task->usedQuantum = 0;
                task->state = STATE_READY;
                if (task->priority == 0) {queue_enqueue(&sched->queue0, task);}
                else if (task->priority == 1) {queue_enqueue(&sched->queue1, task);}
                sched->runningTask = NULL;
                if (sched->verbose) {printf("[T-%4d] QUANTUM_EXPIRED: SENTINEL ID: %d\n", sched->globalTick + 1, task->id);}    
            }
            else if (Apropiado) {
                task->state = STATE_INTERRUPTED;
                if (task->priority == 1) {queue_push_front(&sched->queue1, task);}
                else if (task->priority == 2) {queue_push_front(&sched->queue2, task);}
                sched->runningTask = NULL;
            }
        }

        //Despacho de procesos
        if (sched->runningTask == NULL) {
            if (!queue_is_empty(&sched->queue0)) {sched->runningTask = queue_dequeue(&sched->queue0);}
            else if (!queue_is_empty(&sched->queue1)) {sched->runningTask = queue_dequeue(&sched->queue1);}
            else if (!queue_is_empty(&sched->queue2)) {sched->runningTask = queue_dequeue(&sched->queue2);}
            
            if (sched->runningTask != NULL) {
                if (sched->runningTask->startTime == -1) {
                    sched->runningTask->startTime = sched->globalTick;
                    sched->runningTask->state = STATE_RUNNING;
                }
            }
        }

        // Avanzar reloj
        sched->globalTick++;

    }
    
}

void scheduler_destroy(Scheduler *sched) {
    queue_clear(&sched->queue0);
    queue_clear(&sched->queue1);
    queue_clear(&sched->queue2);

    pthread_mutex_destroy(&sched->lock);
    pthread_barrier_destroy(&sched->barrier);
}