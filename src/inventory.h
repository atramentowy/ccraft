#ifndef INVENTORY_H
#define INVENTORY_H

#include "chunk.h"

typedef struct {
    BlockType slots[4][9];
} Inventory;

void inventory_init(Inventory* inv);

#endif
