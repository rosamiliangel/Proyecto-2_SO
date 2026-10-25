#include <stdio.h>
#include <stdlib.h>
#include "../include/tcb.h"

TCB* tcb_create(int id, int priority, int arrivalTime, int totalBurst) {
    TCB *task = (TCB*) malloc(sizeof(TCB)); // Asignar memoria para un TCB
    if (!task) return NULL;

    task->id = id;
    task->priority = priority;
    task->arrivalTime = arrivalTime;
    task->totalBurst = totalBurst;
    task->remainingBurst = totalBurst;
    task->usedQuantum = 0;
    task->state = STATE_NEW;
    
    task->startTime = -1;
    task->finishTime = -1;
    task->waitTime = 0;
    task->returnTime = 0;

    return task;
}

// Liberar memoria de un TCB
void tcb_destroy(TCB *task) {
    if (task) {
        free(task);
    }
}