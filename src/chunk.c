#include "chunk.h"
#include "world.h"

#include <glad/glad.h>
#include <cglm/cglm.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

void add_face(Chunk* chunk, vec3 block_pos, int face, BlockType block_type) {
	const float tile_width = 1.0f / 3.0f;
    const float tile_height = 1.0f / 2.0f;

    vec2 base_uv = { tile_width * (block_type % 3), tile_height * (block_type / 3) };

    vec2 uv_offsets[4] = {
        {0.0f, 0.0f},
        {tile_width, 0.0f},
        {tile_width, tile_height},
        {0.0f, tile_height}
    };

    vec3 face_offsets[6][4] = {
        // +X
        {{0.5f, 0.5f, -0.5f}, {0.5f, 0.5f, 0.5f}, {0.5f, -0.5f, 0.5f}, {0.5f, -0.5f, -0.5f}},
        // -X
        {{-0.5f, 0.5f, 0.5f}, {-0.5f, 0.5f, -0.5f}, {-0.5f, -0.5f, -0.5f}, {-0.5f, -0.5f, 0.5f}},
        // +Y
        {{0.5f, 0.5f, -0.5f}, {-0.5f, 0.5f, -0.5f}, {-0.5f, 0.5f, 0.5f}, {0.5f, 0.5f, 0.5f}},
        // -Y
        {{-0.5f, -0.5f, -0.5f}, {0.5f, -0.5f, -0.5f}, {0.5f, -0.5f, 0.5f}, {-0.5f, -0.5f, 0.5f}},
        // +Z
        {{0.5f, 0.5f, 0.5f}, {-0.5f, 0.5f, 0.5f}, {-0.5f, -0.5f, 0.5f}, {0.5f, -0.5f, 0.5f}},
        // -Z
        {{-0.5f, 0.5f, -0.5f}, {0.5f, 0.5f, -0.5f}, {0.5f, -0.5f, -0.5f}, {-0.5f, -0.5f, -0.5f}}
    };

    chunk->vertices = realloc(chunk->vertices, sizeof(Vertex) * (chunk->vertex_count + 4));
    for (int i = 0; i < 4; ++i) {
        Vertex v;
        glm_vec3_copy(face_offsets[face][i], v.position);
        glm_vec3_add(v.position, block_pos, v.position);

        glm_vec2_add(base_uv, uv_offsets[i], v.uv);
        chunk->vertices[chunk->vertex_count++] = v;
    }

    unsigned int base = (unsigned int)(chunk->vertex_count - 4);
    chunk->indices = realloc(chunk->indices, sizeof(unsigned int) * (chunk->index_count + 6));
    chunk->indices[chunk->index_count++] = base;
    chunk->indices[chunk->index_count++] = base + 1;
    chunk->indices[chunk->index_count++] = base + 2;
    chunk->indices[chunk->index_count++] = base + 2;
    chunk->indices[chunk->index_count++] = base + 3;
    chunk->indices[chunk->index_count++] = base;
}

void chunk_init(Chunk* chunk) {
	memset(chunk->blocks, BLOCK_AIR, sizeof(chunk->blocks));
	for(int x = 0; x < CHUNK_SIZE; x++) {
		chunk->blocks[x][0][0] = BLOCK_GRASS;
	}
	for(int y = 0; y < CHUNK_SIZE; y++) {
		chunk->blocks[0][y][0] = BLOCK_GRASS;
	}
	for(int z = 0; z < CHUNK_SIZE; z++) {
		chunk->blocks[0][0][z] = BLOCK_GRASS;
	}

	chunk->vertices = NULL;
	chunk->indices = NULL;
	chunk->vertex_count = 0;
	chunk->index_count = 0;
	
	chunk->vao = chunk->vbo = chunk->ebo = 0;
	glGenVertexArrays(1, &chunk->vao);
    glGenBuffers(1, &chunk->vbo);
	glGenBuffers(1, &chunk->ebo);
}

void chunk_unload(Chunk* chunk) {
    if (chunk->vao) glDeleteVertexArrays(1, &chunk->vao);
    if (chunk->vbo) glDeleteBuffers(1, &chunk->vbo);
    if (chunk->ebo) glDeleteBuffers(1, &chunk->ebo);

    chunk->vao = chunk->vbo = chunk->ebo = 0;
	free(chunk->vertices);
	free(chunk->indices);
}

void chunk_rebuild(World* world, Chunk* chunk, int cx, int cy, int cz) {
	free(chunk->vertices);
    free(chunk->indices);
    chunk->vertices = NULL;
    chunk->indices = NULL;
    chunk->vertex_count = 0;
    chunk->index_count = 0;

	Chunk* neighbors[DIR_COUNT];
	for (Direction d = 0; d < DIR_COUNT; ++d) {
		// neighbors[d] = NULL;//chunk_get_neighbor(world, chunk, d);
		neighbors[d] = chunk_get_neighbor(world, cx, cy, cz, d);
	}

	for (int x = 0; x < CHUNK_SIZE; ++x) {
   		for (int y = 0; y < CHUNK_SIZE; ++y) {
        	for (int z = 0; z < CHUNK_SIZE; ++z) {
            	BlockType bt = chunk->blocks[x][y][z];
            	if (bt == BLOCK_AIR) continue;

            	vec3 posf = { (float)x, (float)y, (float)z };
				
				// +X
           		if (x == CHUNK_SIZE - 1) {
                	BlockType nb = neighbors[DIR_POS_X] ? neighbors[DIR_POS_X]->blocks[0][y][z] : BLOCK_AIR;
                	if (nb == BLOCK_AIR) add_face(chunk, posf, DIR_POS_X, bt);
            	} else if (chunk->blocks[x+1][y][z] == BLOCK_AIR) {
                	add_face(chunk, posf, DIR_POS_X, bt);
            	}

            	// -X
            	if (x == 0) {
                	BlockType nb = neighbors[DIR_NEG_X] ? neighbors[DIR_NEG_X]->blocks[CHUNK_SIZE-1][y][z] : BLOCK_AIR;
                	if (nb == BLOCK_AIR) add_face(chunk, posf, DIR_NEG_X, bt);
            	} else if (chunk->blocks[x-1][y][z] == BLOCK_AIR) {
                	add_face(chunk, posf, DIR_NEG_X, bt);
            	}

            	// +Y
            	if (y == CHUNK_SIZE - 1) {
                	BlockType nb = neighbors[DIR_POS_Y] ? neighbors[DIR_POS_Y]->blocks[x][0][z] : BLOCK_AIR;
                	if (nb == BLOCK_AIR) add_face(chunk, posf, DIR_POS_Y, bt);
            	} else if (chunk->blocks[x][y+1][z] == BLOCK_AIR) {
                	add_face(chunk, posf, DIR_POS_Y, bt);
            	}

            	// -Y
            	if (y == 0) {
                	BlockType nb = neighbors[DIR_NEG_Y] ? neighbors[DIR_NEG_Y]->blocks[x][CHUNK_SIZE-1][z] : BLOCK_AIR;
                	if (nb == BLOCK_AIR) add_face(chunk, posf, DIR_NEG_Y, bt);
            	} else if (chunk->blocks[x][y-1][z] == BLOCK_AIR) {
                	add_face(chunk, posf, DIR_NEG_Y, bt);
            	}

            	// +Z
            	if (z == CHUNK_SIZE - 1) {
                	BlockType nb = neighbors[DIR_POS_Z] ? neighbors[DIR_POS_Z]->blocks[x][y][0] : BLOCK_AIR;
                	if (nb == BLOCK_AIR) add_face(chunk, posf, DIR_POS_Z, bt);
            	} else if (chunk->blocks[x][y][z+1] == BLOCK_AIR) {
                	add_face(chunk, posf, DIR_POS_Z, bt);
            	}

            	// -Z
            	if (z == 0) {
                	BlockType nb = neighbors[DIR_NEG_Z] ? neighbors[DIR_NEG_Z]->blocks[x][y][CHUNK_SIZE-1] : BLOCK_AIR;
                	if (nb == BLOCK_AIR) add_face(chunk, posf, DIR_NEG_Z, bt);
            	} else if (chunk->blocks[x][y][z-1] == BLOCK_AIR) {
                	add_face(chunk, posf, DIR_NEG_Z, bt);
            	}
        	}
		}
    }

    glBindVertexArray(chunk->vao);

    glBindBuffer(GL_ARRAY_BUFFER, chunk->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * chunk->vertex_count, chunk->vertices, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(vec3)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunk->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * chunk->index_count, chunk->indices, GL_STATIC_DRAW);

    glBindVertexArray(0);
}

void chunk_draw(const Chunk* chunk, Shader* shader) {
	glBindVertexArray(chunk->vao);
	glDrawElements(GL_TRIANGLES, (GLsizei)chunk->index_count, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}
