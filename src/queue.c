#include <stdio.h>
#include <stdlib.h>
#include "../include/queue.h"

void queue_init(Queue *q) {
    q->head = NULL;
    q->tail = NULL;
    q->size = 0;
}

int queue_is_empty(Queue *q) {
    return q->size == 0;
}

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

void queue_clear(Queue *q) {
    while (!queue_is_empty(q)) {
        queue_dequeue(q);
    }
}