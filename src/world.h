#ifndef WORLD_H
#define WORLD_H

#include "chunk.h"
#include "render_context.h"

#define WORLD_SIZE_X 10
#define WORLD_SIZE_Y 10
#define WORLD_SIZE_Z 10
#define MAX_WORLD_SIZE (WORLD_SIZE_X * WORLD_SIZE_Y * WORLD_SIZE_Z)

typedef struct World {
    Chunk* chunks;
} World;

int world_get_chunk_index(int x, int y, int z);

Chunk* chunk_get_neighbor(World* world, int x, int y, int z, Direction dir);
BlockType world_get_block(World* world, int x, int y, int z);
void world_set_block(World* world, int x, int y, int z, BlockType block);

void world_init(World* world);
void world_unload(World* world);
void world_generate(World* world);
void world_update_mesh(World* world);
void world_update_light(World* world);
void world_draw(const RenderContext* ctx, World* world, Shader* shader);

#endif // WORLD_H
