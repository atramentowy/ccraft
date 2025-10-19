#include "block.h"

bool block_is_transparent(BlockType type) {
    switch(type) {
        case BLOCK_AIR:
        case BLOCK_GLASS:
            return true;
        case BLOCK_LIGHT:
        case BLOCK_GRASS:
        case BLOCK_STONE:
            return false;
    }
    return false;
}

bool block_is_opaque(BlockType type) {
    if(type == BLOCK_AIR) return true;
    return false;
}

uint8_t block_get_emission(BlockType type) {
    if (BLOCK_LIGHT) return 15;
    return 0;
}
