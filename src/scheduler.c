#include <stdio.h>
#include <stdlib.h>
#include "../include/scheduler.h"

const char *TiposPrioridad[] = {"Command", "Perforator", "Heavy"};

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
                if(sched->verbose){ printf("[T-%04d] ARRIVAL: Sentinel ID: %d [Type: Command]\n", sched->globalTick, task->id);}
            }else if(task->priority == 1){
                queue_enqueue(&sched->queue1, task);
                if(sched->verbose){ printf("[T-%04d] ARRIVAL: Sentinel ID: %d [Type: Driller]\n", sched->globalTick, task->id);}
            }else if (task->priority == 2){
                queue_enqueue(&sched->queue2, task);
                if(sched->verbose){ printf("[T-%04d] ARRIVAL: Sentinel ID: %d [Type: Heavy]\n", sched->globalTick, task->id); }
            }
        }
    }
    pthread_mutex_unlock(&sched->lock); // Libere la sección crítica

    return NULL;
}

static void* worker_thread_func(void *arg) {
    Scheduler *sched = (Scheduler*) arg;
    // Bucle de simulación del planificador
    while(1){
        // --- Sincronización con la barrera ---
        // El hilo de CPU espera a que el maestro termine de admitir las llegadas
        pthread_barrier_wait(&sched->barrier);
        pthread_mutex_lock(&sched->lock);
        // Protección de la sección crítica
        // Condición de parada: De no haber tareas activas, ni en colas, salir.
        if(sched->runningTask == NULL && queue_is_empty(&sched->queue0) && queue_is_empty(&sched->queue1) && queue_is_empty(&sched->queue2)){
            // Verificar si se han completado todas las tareas
            int llegadasPendientes = 0;
            for(int i = 0; i < sched->totalTasks; i++){
                if(sched->allTasks[i]->state == STATE_NEW){
                    llegadasPendientes = 1;
                    break;
                }
            }
            if(!llegadasPendientes){
                pthead_mutex_unlock(&sched->lock);
                break; // Salir del bucle y terminar el hilo
            }
        }
        // --- Exprociación ---
        // Si hay una tarea ejecutándose en una cola de menor prioridad y llega algo de una cola de mayor prioridad
        if(sched->runningTask != NULL && sched->runningTask->priority > 0){
            int mayorPrioridad = 0;
            if(sched->runningTask->priority == 2 && (!queue_is_empty(&sched->queue0) || !queue_is_empty(&sched->queue1))){ mayorPrioridad = 1;
            }else if(sched->runningTask->priority == 1 && !queue_is_empty(&sched->queue0)){ mayorPrioridad = 1;}
            // Si hay una tarea de mayor prioridad en la cola
            if(mayorPrioridad){
                TCB *interrumpir = sched->runningTask;
                interrumpir->state = STATE_INTERRUPTED;
                // Reingresa a la cabeza de su respectiva cola conservando su progreso
                if(interrumpir->priority == 1){ queue_push_front(&sched->queue1, interrumpir);
                }else if(interrumpir->priority == 2){ queue_push_front(&sched->queue2, interrumpir); }
                // Informar de Interrupción
                if(sched->verbose){ printf("[T-%04d] PREEMPTED: Sentinel ID: %d displaced by higher priority queue\n", sched->globalTick, interrumpir->id); }
                sched->runningTask = NULL;
            }
        }
        // --- Despacho ---
        if(sched->runningTask == NULL){
            if(!queue_is_empty(&sched->queue0)){ sched->runningTask = queue_dequeue(&sched->queue0);
            }else if(!queue_is_empty(&sched->queue1)){ sched->runningTask = queue_dequeue(&sched->queue1);
            }else if(!queue_is_empty(&sched->queue2)){ sched->runningTask = queue_dequeue(&sched->queue2);}
        }

        if(sched->runningTask != NULL){
                sched->runningTask->state = STATE_RUNNING;
                if(sched->runningTask->startTime == -1){ sched->runningTask->startTime = sched->globalTick; }
        }

        // --- Ejecución y expira el QUANTUM ---
        if (sched->runningTask != NULL) {
            TCB *task = sched->runningTask;
            task->remainingBurst--;
            task->usedQuantum++;

            if(sched->verbose){ printf("[T-%04d] RUNNING: Sentinel ID: %d (Remaining Burst: %d, Quantum: %d)\n", sched->globalTick, task->id, task->remainingBurst, task->usedQuantum); }
            // Evaluar finalización
            if(task->remainingBurst == 0){
                task->state = STATE_FINISHED;
                task->finishTime = sched->globalTick + 1;
                task->returnTime = task->finishTime - task->arrivalTime;
                // Si se completa
                if(sched->verbose || 1){ printf("[T-%04d] COMPLETED: Sentinel ID: %d -> Turnaround Time: %d ticks\n", sched->globalTick + 1, task->id, task->returnTime); }
                sched->runningTask = NULL;
            }else{
                // Evaluar expiración de Quantum (Round Robin para Cola 0 y Cola 1)
                int limiteQuantum = (task->priority == 0) ? 2 : (task->priority == 1) ? 4 : 0;
                if (task->priority != 2 && task->usedQuantum >= limiteQuantum) {
                    task->usedQuantum = 0;
                    task->state = STATE_READY;
                    if (task->priority == 0) { queue_enqueue(&sched->queue0, task);
                    }else{ queue_enqueue(&sched->queue1, task); }
                    sched->runningTask = NULL;
                }
            }
        }
        // --- Avance de Reloj ---
        sched->globalTick++;
        pthread_mutex_unlock(&sched->lock);
        // Termina la sección crítica
    }
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
    pthread_t hiloMaestro, hiloTrabajador;

    // Crear los dos hilos requeridos
    pthread_create(&hiloMaestro, NULL, master_thread_func, sched);
    pthread_create(&hiloTrabajador, NULL, worker_thread_func, sched);

    // Esperar a que terminen la simulación
    pthread_join(hiloMaestro, NULL);
    pthread_join(hiloTrabajador, NULL);

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

void scheduler_print_metrics(Scheduler *sched){
    int tareasTotales = sched->totalTasks;
    int tiempoTotal = sched->globalTick; // Valor final de Tg
    float rendimiento = (float) tareasTotales / tiempoTotal;
    float tiempoDeEsperaTotal = 0;

    for(int i = 0; i < tareasTotales; i++){
        TCB *tarea = sched->allTasks[i];
        // Tiempo de espera por tarea
        int tiempoDeESpera = tarea->returnTime - tarea->totalBurst;
        tiempoDeEsperaTotal += tiempoDeESpera;
    }
    // Calcular el tiempo de espera promedio
    float tiempoDeEsperaPromedio = tiempoDeEsperaTotal / tareasTotales;
    // Mostrar las metricas
    printf("\n====== GLOBAL PERFORMANCE METRICS ======\n");
    printf("Total Simulation Time : %d clock ticks.\n", tiempoTotal);
    printf("System Throughput : %.2f sentinels/tick.\n", rendimiento);
    printf("Average Waiting Time : %.2f ticks.\n", tiempoDeEsperaPromedio);
}

void scheduler_destroy(Scheduler *sched) {
    queue_clear(&sched->queue0);
    queue_clear(&sched->queue1);
    queue_clear(&sched->queue2);
    
    pthread_mutex_destroy(&sched->lock);
    pthread_barrier_destroy(&sched->barrier);
}