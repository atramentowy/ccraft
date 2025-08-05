#ifndef WORLD_H
#define WORLD_H

#include "chunk.h"

#define WORLD_SIZE_X  3
#define WORLD_SIZE_Y  3
#define WORLD_SIZE_Z 3

typedef struct World {
    Chunk chunks[WORLD_SIZE_X][WORLD_SIZE_Y][WORLD_SIZE_Z];
} World;

// int chunk_index(int x, int y, int z);
// void chunk_coords_from_index(int index, int* x, int* y, int* z);
// Chunk* world_get_chunk_at(World* world, int x, int y, int z);
Chunk* chunk_get_neighbor(World* world, int x, int y, int z, Direction dir);

void world_init(World* world);
void world_unload(World* world);
void world_generate(World* world);
void world_rebuild(World* world);
void world_draw(const World* world, Shader* shader);

#endif // WORLD_H
