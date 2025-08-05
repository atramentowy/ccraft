#ifndef CHUNK_H
#define CHUNK_H

#include <glad/glad.h>
#include <cglm/cglm.h>

#include "shader.h"

typedef struct World World;

#define CHUNK_SIZE 16

typedef enum {
	DIR_POS_X = 0,	// +X
	DIR_NEG_X = 1,	// -X
	DIR_POS_Y = 2,	// +Y
	DIR_NEG_Y = 3,	// -Y
	DIR_POS_Z = 4,	// +Z
	DIR_NEG_Z = 5,	// -Z
	DIR_COUNT = 6	// COUNT
} Direction;

typedef enum {
    BLOCK_AIR = 0,
	BLOCK_DIRT,
    BLOCK_GRASS,
	BLOCK_STONE
} BlockType;

typedef struct {
	vec3 position;
	vec2 uv;
} Vertex;

typedef struct {
    BlockType blocks[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];

	Vertex* vertices;
	unsigned int* indices;
	size_t vertex_count;
	size_t index_count;

	GLuint vao;
	GLuint vbo;
	GLuint ebo;
} Chunk;

void chunk_init(Chunk* chunk);
void chunk_unload(Chunk* chunk);
void chunk_rebuild(World* world, Chunk* chunk, int cx, int cy, int cz);
void chunk_draw(const Chunk* chunk, Shader* shader);

// chunk_set_block(Chunk* chunk, vec3 pos, BlockType block_type);

#endif
