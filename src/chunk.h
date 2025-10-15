#ifndef CHUNK_H
#define CHUNK_H

#include <glad.h>
#include <cglm/cglm.h>
#include <stdbool.h>

#include "block.h"
#include "shader.h"

typedef struct World World;

#define CHUNK_SIZE 16
#define MAX_QUEUE (CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE)

typedef enum {
	DIR_POS_X = 0,	// +X
	DIR_NEG_X = 1,	// -X
	DIR_POS_Y = 2,	// +Y
	DIR_NEG_Y = 3,	// -Y
	DIR_POS_Z = 4,	// +Z
	DIR_NEG_Z = 5,	// -Z
	DIR_COUNT = 6	// COUNT
} Direction;

typedef struct {
	vec3 position;
	vec2 uv;
    float light;
} Vertex;

typedef struct {
    int x, y, z;
    uint8_t light;
} LightNode;

typedef struct Chunk {
	Block* blocks;

	Vertex* vertices;
	unsigned int* indices;
	size_t vertex_count;
	size_t index_count;

    LightNode queue[MAX_QUEUE];
    int q_front, q_back;

	GLuint vao;
	GLuint vbo;
	GLuint ebo;

	bool dirty;
	bool visible;
} Chunk;

int chunk_get_block_index(int x, int y, int z);

BlockType chunk_get_block(Chunk* chunk, int x, int y, int z);
void chunk_set_block(Chunk* chunk, int x, int y, int z, BlockType block);

void chunk_init(Chunk* chunk);
void chunk_unload(Chunk* chunk);
void chunk_update_mesh(World* world, Chunk* chunk, int cx, int cy, int cz); // update mesh
void chunk_update_light(World* world, Chunk* chunk); // update light
void chunk_draw(const Chunk* chunk, Shader* shader);

#endif

