#include "inventory.h"

void inventory_init(Inventory* inv) {
    for(int i = 0; i < 8; i++) {
        for(int j = 0; j < 3; j++) {
            inv->slots[i][j] = BLOCK_LIGHT; // todo: change the block list
        }
    }
}
