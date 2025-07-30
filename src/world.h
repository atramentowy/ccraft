#ifndef WORLD_H
#define WORLD_H

#include <glad/glad.h>
#include <cglm/cglm.h>

#include "chunk.h"

#define WORLD_SIZE_X  3
#define WORLD_SIZE_Y  3
#define WORLD_SIZE_Z  3

typedef struct {
    Chunk chunks[WORLD_SIZE_X][WORLD_SIZE_Y][WORLD_SIZE_Z];
} World;

void world_init(World* world);
void world_unload(World* world);
void world_generate(World* world);
void world_rebuild(World* world);
void world_draw(const World* world, Shader* shader);

#endif // WORLD_H

