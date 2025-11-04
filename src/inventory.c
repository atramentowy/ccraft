#include "inventory.h"

void inventory_init(Inventory* inv) {
    for(int x = 0; x < 3; x++) {
        for(int y = 0; y < 8; y++) {
            inv->slots[x][y] = BLOCK_LIGHT;
        }
    }
}
