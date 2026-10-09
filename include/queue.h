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

void queue_init(Queue *q);
int queue_is_empty(Queue *q);
void queue_enqueue(Queue *q, TCB *task);      // Al final de la cola
void queue_push_front(Queue *q, TCB *task);   // Al frente (para expropiaciones)
TCB* queue_dequeue(Queue *q);
void queue_clear(Queue *q);

#endif