#include "block.h"

#define CHUNK_SIZE 16

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

bool block_in_chunk(ivec3 pos) { // does not check in world coords
    return (
        pos[0] >= 0 && pos[0] < CHUNK_SIZE &&
        pos[1] >= 0 && pos[1] < CHUNK_SIZE &&
        pos[2] >= 0 && pos[2] < CHUNK_SIZE
    ); 
}

uint8_t block_get_emission(BlockType type) {
    if (BLOCK_LIGHT) return 15;
    return 0;
}
