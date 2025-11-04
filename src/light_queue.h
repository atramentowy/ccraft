#ifndef LIGHT_QUEUE_H
#define LIGHT_QUEUE_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    int x, y, z;
    uint8_t light;
} LightNode;

#define LIGHT_QUEUE_CAPACITY 65536  // must be the power of two
#define LIGHT_QUEUE_MASK (LIGHT_QUEUE_CAPACITY - 1) // bitmask

typedef struct {
    LightNode data[LIGHT_QUEUE_CAPACITY];
    uint32_t head;
    uint32_t tail;
} LightQueue;

void lightqueue_init(LightQueue *q);
bool lightqueue_empty(const LightQueue *q);
bool lightqueue_full(const LightQueue *q);
void lightqueue_push(LightQueue *q, LightNode node);
LightNode lightqueue_pop(LightQueue *q);
uint32_t lightqueue_count(const LightQueue *q);

#endif // LIGHT_QUEUE_H

