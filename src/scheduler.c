#include <stdio.h>
#include <stdlib.h>
#include "../include/scheduler.h"

const char *TiposPrioridad[] = {"Command", "Driller", "Heavy"};

static void* master_thread_func(void *arg) {
    Scheduler *sched = (Scheduler*) arg;

    while(1) {
        pthread_mutex_lock(&sched->lock); // Protección de la sección crítica

        //condicion de parada
        int llegadasPendientes = 0;
        for(int i = 0; i < sched->totalTasks; i++){
            if(sched->allTasks[i]->state == STATE_NEW){ llegadasPendientes = 1; break; }
        }
        if(sched->runningTask == NULL && queue_is_empty(&sched->queue0) && queue_is_empty(&sched->queue1) && queue_is_empty(&sched->queue2) && !llegadasPendientes){
            pthread_mutex_unlock(&sched->lock);
            pthread_barrier_wait(&sched->barrier); // <-- ¡Línea obligatoria para liberar al hilo trabajador!
            pthread_barrier_wait(&sched->barrier_end); // Sincronización de salida
            break;
        }

        // LLegada de procesos y encolamiento según prioridad
        for (int i = 0; i < sched->totalTasks; i++) {
            TCB *task = sched->allTasks[i];
            if (task->arrivalTime == sched->globalTick && task->state == STATE_NEW) {
                task->state = STATE_READY;

                if (task->priority == 0) {queue_enqueue(&sched->queue0,task);}
                else if (task->priority == 1) {queue_enqueue(&sched->queue1,task);}
                else if (task->priority == 2) {queue_enqueue(&sched->queue2,task);}

                // Salida estandar
                printf("[T-%04d] ARRIVAL: Sentinel ID: %d [Type: %s]\n", sched->globalTick, task->id, TiposPrioridad[task->priority]);
                // Salida ampliada
                //if (sched->verbose) {}
            }
        }
        
        pthread_mutex_unlock(&sched->lock); // Libere la sección crítica
        pthread_barrier_wait(&sched->barrier); // Terminar de agregar procesos, despertar hilo trabajador
        pthread_barrier_wait(&sched->barrier_end); //Esperar a que se termine el procesamiento del tick
    }
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
        int llegadasPendientes = 0;
        for(int i = 0; i < sched->totalTasks; i++){
            if(sched->allTasks[i]->state == STATE_NEW){ llegadasPendientes = 1; break; }
        }
        if(sched->runningTask == NULL && queue_is_empty(&sched->queue0) && queue_is_empty(&sched->queue1) && queue_is_empty(&sched->queue2) && !llegadasPendientes){
            pthread_mutex_unlock(&sched->lock);
            pthread_barrier_wait(&sched->barrier_end); // Sincronización de salida
            break;
        }

        // Ejecución de procesos
        if (sched->runningTask !=NULL) {
            TCB *task = sched->runningTask;

            int LimiteQuantum = (task->priority == 0) ? 2 : ((task->priority == 1) ? 4: 0);

            // Expiración por finalización
            if (task->remainingBurst == 0) {
                task->state = STATE_FINISHED;
                task->finishTime = sched->globalTick;
                task->returnTime = task->finishTime - task->arrivalTime;
                sched->runningTask = NULL;
                
                //salida ampliada
                //if (sched->verbose) {}
                //Salida estandar
                printf("[T-%04d] COMPLETED: Sentinel ID: %d -> Turnaround Time: %d ticks\n", sched->globalTick, task->id, task->returnTime);
            }
            // Expiración por quantum
            else if (LimiteQuantum >0 && task->usedQuantum == LimiteQuantum) {
                int alternar = 0;

                //Verificar si existen procesos en la misma cola
                if (task->priority == 0 && !queue_is_empty(&sched->queue0)) alternar = 1;
                if (task->priority == 1 && !queue_is_empty(&sched->queue1)) alternar = 1;
                
                // Alternar si hay varios procesos en una sola cola
                if (alternar) {
                    task->usedQuantum = 0;
                    task->state = STATE_READY;

                    if (task->priority == 0) {queue_enqueue(&sched->queue0, task);}
                    else if (task->priority == 1) { queue_enqueue(&sched->queue1, task);}

                    sched->runningTask = NULL; // Liberar CPU para el siguiente proceso
                    
                    //Salida ampliada
                    //if (sched->verbose) {}
                    // Salida estandar
                    printf("[T-%04d] QUANTUM_EXPIRED: SENTINEL %d yields CPU via RR rotation (Q=%d)\n", sched->globalTick, task->id, LimiteQuantum);
                } else {
                    // Reiniciar quantum si no hay mas procesos en la misma cola
                    task->usedQuantum = 0;
                } 
            }
        }

        // Expropiación del CPU
        if (sched->runningTask != NULL) {
            TCB *task = sched->runningTask;
            int Apropiado = 0;
            if (task->priority > 0 && !queue_is_empty(&sched->queue0)) { Apropiado = 1; }
            if (task->priority == 2 && !queue_is_empty(&sched->queue1)) { Apropiado = 1; }

            if (Apropiado) {
                task->state = STATE_INTERRUPTED;
                if (task->priority == 1) { queue_push_front(&sched->queue1, task); }
                else if (task->priority == 2) { queue_push_front(&sched->queue2, task); }
                sched->runningTask = NULL;
            }
        }

        // Despacho de procesos
        if (sched->runningTask == NULL) {
            if (!queue_is_empty(&sched->queue0)) {sched->runningTask = queue_dequeue(&sched->queue0);}
            else if (!queue_is_empty(&sched->queue1)) {sched->runningTask = queue_dequeue(&sched->queue1);}
            else if (!queue_is_empty(&sched->queue2)) {sched->runningTask = queue_dequeue(&sched->queue2);}
            
            if (sched->runningTask != NULL) {
                if (sched->runningTask->startTime == -1) {
                    sched->runningTask->startTime = sched->globalTick; 
                }
                sched->runningTask->state = STATE_RUNNING;
            }
        }

        // Terminar tic actual
        if (sched->runningTask != NULL) {
            sched->runningTask->usedQuantum++;
            sched->runningTask->remainingBurst--;
        }

        // Avanzar reloj
        sched->globalTick++;
        pthread_mutex_unlock(&sched->lock);
        pthread_barrier_wait(&sched->barrier_end); // Cierre síncrono del tic
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
    pthread_barrier_init(&sched->barrier_end, NULL, 2);
}

void scheduler_run(Scheduler *sched) {
    pthread_t hiloMaestro, hiloTrabajador;

    // Crear los dos hilos requeridos
    pthread_create(&hiloMaestro, NULL, master_thread_func, sched);
    pthread_create(&hiloTrabajador, NULL, worker_thread_func, sched);

    // Esperar a que terminen la simulación
    pthread_join(hiloMaestro, NULL);
    pthread_join(hiloTrabajador, NULL);

    printf("Simulación finalizada correctamente.\n");
}

void scheduler_print_metrics(Scheduler *sched){
    int tareasTotales = sched->totalTasks;

    // Calcular el tiempo total evaluando el último finishTime
    int tiempoTotal = 0;
    for(int i = 0; i < tareasTotales; i++){
        if(sched->allTasks[i]->finishTime > tiempoTotal){tiempoTotal = sched->allTasks[i]->finishTime;}
    }

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
    pthread_barrier_destroy(&sched->barrier_end);
}