#ifndef WORLD_H
#define WORLD_H

#include "chunk.h"
#include "render_context.h"

#define WORLD_SIZE_X 5
#define WORLD_SIZE_Y 5
#define WORLD_SIZE_Z 5

typedef struct World {
    Chunk chunks[WORLD_SIZE_X][WORLD_SIZE_Y][WORLD_SIZE_Z];
} World;

Chunk* chunk_get_neighbor(World* world, int x, int y, int z, Direction dir);

void world_init(World* world);
void world_unload(World* world);
void world_generate(World* world);
void world_rebuild(World* world);
void world_draw(const RenderContext* ctx, World* world, Shader* shader);

#endif // WORLD_H
