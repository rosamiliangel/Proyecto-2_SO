#include <stdio.h>
#include <stdlib.h>
#include "../include/queue.h"

// Inicializar la cola
void queue_init(Queue *q) {
    q->head = NULL;
    q->tail = NULL;
    q->size = 0;
}

// Verificar si la cola está vacía
int queue_is_empty(Queue *q) {
    return q->size == 0;
}

// Insertar al final de la cola
void queue_enqueue(Queue *q, TCB *task) {
    Node *node = (Node*) malloc(sizeof(Node));
    node->task = task;
    node->next = NULL;

    if (queue_is_empty(q)) {
        q->head = node;
        q->tail = node;
    } else {
        q->tail->next = node;
        q->tail = node;
    }
    q->size++;
}

// Insertar al frente de la cola (para expropiaciones)
void queue_push_front(Queue *q, TCB *task) {
    Node *node = (Node*) malloc(sizeof(Node));
    node->task = task;
    node->next = q->head;

    q->head = node;
    if (q->tail == NULL) {
        q->tail = node;
    }
    q->size++;
}

// Eliminar del frente de la cola y liberar memoria del nodo
TCB* queue_dequeue(Queue *q) {
    if (queue_is_empty(q)) return NULL;

    Node *temp = q->head;
    TCB *task = temp->task;

    q->head = q->head->next;
    if (q->head == NULL) {
        q->tail = NULL;
    }

    free(temp);
    q->size--;
    return task;
}

// vaciar cola 
void queue_clear(Queue *q) {
    while (!queue_is_empty(q)) {
        queue_dequeue(q);
    }
}