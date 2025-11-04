#include "light_queue.h"

void lightqueue_init(LightQueue *q) {
    q->head = 0;
    q->tail = 0;
}

bool lightqueue_empty(const LightQueue *q) {
    return q->head == q->tail;
}

bool lightqueue_full(const LightQueue *q) { 
    return ((q->tail + 1) & LIGHT_QUEUE_MASK) == q->head;
}

void lightqueue_push(LightQueue *q, LightNode node) {
    // no overflow check for max performance (handle externally)
    q->data[q->tail] = node;
    q->tail = (q->tail + 1) & LIGHT_QUEUE_MASK;
}

LightNode lightqueue_pop(LightQueue *q) {
    LightNode node = q->data[q->head];
    q->head = (q->head + 1) & LIGHT_QUEUE_MASK;
    return node;
}

uint32_t lightqueue_count(const LightQueue *q) {
    return (q->tail - q->head) & LIGHT_QUEUE_MASK;
}

