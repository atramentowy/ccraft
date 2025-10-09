#include "chunk.h"
#include "world.h"

#include <cglm/cglm.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

void add_face(Chunk* chunk, vec3 block_pos, int face, Block block_type) {
    const float tile_size = 1.0f / 16.0f; // 16x16 tiles
    const float epsilon = 0.001f;

    int tile_x = block_type % 16;
    int tile_y = block_type / 16;

    tile_y = 15 - tile_y;

    // Base UV (bottom-left of tile)
    vec2 base_uv = {
        tile_x * tile_size + epsilon,
        tile_y * tile_size + epsilon
    };

    vec2 uv_offsets[4] = {
        {0.0f, 0.0f},
        {tile_size - 2 * epsilon, 0.0f},
        {tile_size - 2 * epsilon, tile_size - 2 * epsilon},
        {0.0f, tile_size - 2 * epsilon}
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

    // Reallocate vertex buffer
    Vertex* new_vertices = realloc(chunk->vertices, sizeof(Vertex) * (chunk->vertex_count + 4));
    if (!new_vertices) {
        fprintf(stderr, "Failed to realloc vertices\n");
        return;
    }
    chunk->vertices = new_vertices;

    // Add 4 vertices for the face
    for (int i = 0; i < 4; ++i) {
        Vertex v;
        glm_vec3_copy(face_offsets[face][i], v.position);
        glm_vec3_add(v.position, block_pos, v.position);

        glm_vec2_add(base_uv, uv_offsets[i], v.uv);
        chunk->vertices[chunk->vertex_count++] = v;
    }

    // Reallocate index buffer
    unsigned int* new_indices = realloc(chunk->indices, sizeof(unsigned int) * (chunk->index_count + 6));
    if (!new_indices) {
        fprintf(stderr, "Failed to realloc indices\n");
        return;
    }
    chunk->indices = new_indices;

    unsigned int base = chunk->vertex_count - 4;

    // Add 2 triangles (quad)
    chunk->indices[chunk->index_count++] = base;
    chunk->indices[chunk->index_count++] = base + 1;
    chunk->indices[chunk->index_count++] = base + 2;
    chunk->indices[chunk->index_count++] = base + 2;
    chunk->indices[chunk->index_count++] = base + 3;
    chunk->indices[chunk->index_count++] = base;
}

int chunk_get_block_index(int x, int y, int z) {
    if (x < 0 || x >= CHUNK_SIZE || y < 0 || y >= CHUNK_SIZE || z < 0 || z >= CHUNK_SIZE) {
        return -1;
    }
    return x * CHUNK_SIZE * CHUNK_SIZE + y * CHUNK_SIZE + z;
}

Block chunk_get_block(Chunk* chunk, int x, int y, int z) {
	if (x < 0 || x >= CHUNK_SIZE ||
		y < 0 || y >= CHUNK_SIZE ||
		z < 0 || z >= CHUNK_SIZE) {
		return BLOCK_AIR;
	}

	return chunk->blocks[chunk_get_block_index(x, y, z)];
}

void chunk_set_block(Chunk* chunk, int x, int y, int z, Block block) {
	if (x < 0 || x >= CHUNK_SIZE ||
        y < 0 || y >= CHUNK_SIZE ||
        z < 0 || z >= CHUNK_SIZE) {
        return;
    }

    chunk->blocks[chunk_get_block_index(x, y, z)] = block;
}

void chunk_init(Chunk* chunk) {
	chunk->blocks = malloc(CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * sizeof(Block));
	memset(chunk->blocks, BLOCK_AIR, CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * sizeof(Block));

	// chunk->blocks[chunk_get_block_index(0, 0, 0)] = BLOCK_GRASS;
	chunk_set_block(chunk, 0, 0, 0, BLOCK_GRASS);

	chunk->vertices = NULL;
	chunk->indices = NULL;
	chunk->vertex_count = 0;
	chunk->index_count = 0;
	
	chunk->vao = chunk->vbo = chunk->ebo = 0;
	glGenVertexArrays(1, &chunk->vao);
    glGenBuffers(1, &chunk->vbo);
	glGenBuffers(1, &chunk->ebo);

	chunk->dirty = true;
	chunk->visible = false;
}

void chunk_unload(Chunk* chunk) {
    // Free dynamically allocated block data
    free(chunk->blocks);

    if (chunk->vao)
        glDeleteVertexArrays(1, &chunk->vao);
    if (chunk->vbo)
        glDeleteBuffers(1, &chunk->vbo);
    if (chunk->ebo)
        glDeleteBuffers(1, &chunk->ebo);

    chunk->vao = 0;
    chunk->vbo = 0;
    chunk->ebo = 0;

    // Free dynamically allocated mesh data
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

	Chunk* neighbors[DIR_COUNT] = {0};
	for (Direction d = 0; d < DIR_COUNT; ++d) {
		neighbors[d] = chunk_get_neighbor(world, cx, cy, cz, d);
	}

	for (int x = 0; x < CHUNK_SIZE; ++x) {
   		for (int y = 0; y < CHUNK_SIZE; ++y) {
        	for (int z = 0; z < CHUNK_SIZE; ++z) {
				Block bt = chunk->blocks[chunk_get_block_index(x, y, z)];
            	if (bt == BLOCK_AIR) continue;

            	vec3 posf = { (float)x, (float)y, (float)z };
				
				// +X
           		if (x == CHUNK_SIZE - 1) {
                	Block nb = neighbors[DIR_POS_X] 
                        ? neighbors[DIR_POS_X]->blocks[chunk_get_block_index(0, y, z)] : 0;
                	if(nb == BLOCK_AIR || nb != bt)
                        add_face(chunk, posf, DIR_POS_X, bt);
            	} else {
                    Block nb = chunk->blocks[chunk_get_block_index(x+1, y, z)];
                    if(nb == BLOCK_AIR || nb != bt)
                        add_face(chunk, posf, DIR_POS_X, bt);
            	}

            	// -X
            	if (x == 0) {
                	Block nb = neighbors[DIR_NEG_X] 
                        ? neighbors[DIR_NEG_X]->blocks[chunk_get_block_index(CHUNK_SIZE-1, y, z)] : 0;
                	if (nb == BLOCK_AIR || nb != bt)
                        add_face(chunk, posf, DIR_NEG_X, bt);
            	} else {
                    Block nb = chunk->blocks[chunk_get_block_index(x-1, y, z)];
                    if(nb == BLOCK_AIR || nb != bt)
                        add_face(chunk, posf, DIR_NEG_X, bt);
            	}

            	// +Y
            	if (y == CHUNK_SIZE - 1) {
                	Block nb = neighbors[DIR_POS_Y] 
                        ? neighbors[DIR_POS_Y]->blocks[chunk_get_block_index(x, 0, z)] : 0;
                	if (nb == BLOCK_AIR || nb != bt)
                        add_face(chunk, posf, DIR_POS_Y, bt);
            	} else {
                    Block nb = chunk->blocks[chunk_get_block_index(x, y+1, z)];
                    if(nb == BLOCK_AIR || nb != bt)
                        add_face(chunk, posf, DIR_POS_Y, bt);
            	}

            	// -Y
            	if (y == 0) {
                	Block nb = neighbors[DIR_NEG_Y] 
                        ? neighbors[DIR_NEG_Y]->blocks[chunk_get_block_index(x, CHUNK_SIZE-1, z)] : 0;
                	if (nb == BLOCK_AIR || nb != bt)
                        add_face(chunk, posf, DIR_NEG_Y, bt);
            	} else {
                    Block nb = chunk->blocks[chunk_get_block_index(x, y-1, z)];
                    if(nb == BLOCK_AIR || nb != bt)
                        add_face(chunk, posf, DIR_NEG_Y, bt);
            	}

            	// +Z
            	if (z == CHUNK_SIZE - 1) {
                	Block nb = neighbors[DIR_POS_Z] 
                        ? neighbors[DIR_POS_Z]->blocks[chunk_get_block_index(x, y, 0)] : 0;
                	if (nb == BLOCK_AIR || nb != bt)
                        add_face(chunk, posf, DIR_POS_Z, bt);
            	} else {
                    Block nb = chunk->blocks[chunk_get_block_index(x, y, z+1)];
                	if(nb == BLOCK_AIR || nb != bt)
                        add_face(chunk, posf, DIR_POS_Z, bt);
            	}

            	// -Z
            	if (z == 0) {
                	Block nb = neighbors[DIR_NEG_Z] 
                        ? neighbors[DIR_NEG_Z]->blocks[chunk_get_block_index(x, y, CHUNK_SIZE-1)] : 0;
                	if (nb == BLOCK_AIR || nb != bt)
                        add_face(chunk, posf, DIR_NEG_Z, bt);
            	} else {
                    Block nb = chunk->blocks[chunk_get_block_index(x, y, z-1)];
                    if(nb == BLOCK_AIR || nb != bt)
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
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * chunk->index_count, chunk->indices, GL_DYNAMIC_DRAW);

    glBindVertexArray(0);

	chunk->dirty = false;
}

void chunk_draw(const Chunk* chunk, Shader* shader) {
	glBindVertexArray(chunk->vao);
	glDrawElements(GL_TRIANGLES, (GLsizei)chunk->index_count, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}
