#ifndef TCB_H
#define TCB_H

typedef enum {
    STATE_NEW,
    STATE_READY,
    STATE_RUNNING,
    STATE_INTERRUPTED,
    STATE_FINISHED
} TaskState;

typedef struct {
    int id;              // ID del centinela
    int priority;        // 0: Comando, 1: Perforador, 2: Pesado
    int arrivalTime;     // Tic de llegada
    int totalBurst;      // Ráfaga total de CPU
    int remainingBurst;  // Ráfaga restante
    int usedQuantum;     // Quantum consumido en turno actual
    
    TaskState state;     // Estado de la tarea
    
    // Métricas
    int startTime;
    int finishTime;
    int waitTime;
    int returnTime;
} TCB;

TCB* tcb_create(int id, int priority, int arrivalTime, int totalBurst);
void tcb_destroy(TCB *task);

#endif