#ifndef BLOCK_H
#define BLOCK_H

#include <stdbool.h>
#include <stdint.h>

typedef enum BlockType {
    BLOCK_AIR = 0,
	BLOCK_LIGHT,
    BLOCK_GLASS,
    BLOCK_GRASS,
	BLOCK_STONE
} BlockType;

typedef struct Block {
    BlockType type;
    uint8_t light_level; // 0–15 range (4 bits) 
} Block;

bool block_is_transparent(BlockType type);
bool block_is_opaque(BlockType type);
uint8_t block_get_emission(BlockType type);

#endif
