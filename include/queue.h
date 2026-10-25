#ifndef QUEUE_H
#define QUEUE_H

#include "tcb.h"

typedef struct Node {
    TCB *task;
    struct Node *next;
} Node;

typedef struct {
    Node *head;
    Node *tail;
    int size;
} Queue;

void queue_init(Queue *q);                    // Inicializa la cola
int queue_is_empty(Queue *q);                 // Verificar si la cola está vacía
void queue_enqueue(Queue *q, TCB *task);      // Insertar al final de la cola
void queue_push_front(Queue *q, TCB *task);   // Insertar al frente de la cola (para expropiaciones)
TCB* queue_dequeue(Queue *q);                 // Eliminar del frente de la cola y liberar memoria del nodo
void queue_clear(Queue *q);                   // vaciar cola  

#endif