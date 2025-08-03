#ifndef CHUNK_H
#define CHUNK_H

#include <glad/glad.h>
#include <cglm/cglm.h>

#include "shader.h"

#define CHUNK_SIZE 16

typedef enum {
    Air,
    Grass
} BlockType;

typedef struct {
    BlockType blocks[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];

    GLuint vao, vbo, ebo;

} Chunk;

void chunk_init(Chunk* chunk);
void chunk_unload(Chunk* chunk);
void chunk_rebuild(Chunk* chunk);
void chunk_draw(const Chunk* chunk, Shader* shader);

#endif
