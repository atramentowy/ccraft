#include "chunk.h"
#include "world.h"

#include <glad/glad.h>
#include <cglm/cglm.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

void add_face(Chunk* chunk, vec3 block_pos, int face, Block block_type) {
	const float tile_width = 1.0f / 4.0f;
    const float tile_height = 1.0f / 2.0f;
	
	const float epsilon = 0.001f;
    vec2 base_uv = { 
		tile_width * (block_type % 4) + epsilon,
		tile_height * (block_type / 2) + epsilon
	};

	/*
    vec2 uv_offsets[4] = {
        {0.0f, 0.0f},
        {tile_width, 0.0f},
        {tile_width, tile_height},
        {0.0f, tile_height}
    };
	*/
	vec2 uv_offsets[4] = {
    	{0.0f, 0.0f},
    	{tile_width - 2 * epsilon, 0.0f},
    	{tile_width - 2 * epsilon, tile_height - 2 * epsilon},
    	{0.0f, tile_height - 2 * epsilon}
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

    Vertex* new_vertices = realloc(chunk->vertices, sizeof(Vertex) * (chunk->vertex_count + 4));
	if (!new_vertices) {
		fprintf(stderr, "Failed to realloc vertices\n");
		return;
	}
	chunk->vertices = new_vertices;

    for (int i = 0; i < 4; ++i) {
        Vertex v;
        glm_vec3_copy(face_offsets[face][i], v.position);
        glm_vec3_add(v.position, block_pos, v.position);

        glm_vec2_add(base_uv, uv_offsets[i], v.uv);
        chunk->vertices[chunk->vertex_count++] = v;
    }

    unsigned int base = (unsigned int)(chunk->vertex_count - 4);


    unsigned int* new_indices = realloc(chunk->indices, sizeof(unsigned int) * (chunk->index_count + 6));
	if (!new_indices) {
		fprintf(stderr, "Failed to realloc indices\n");
		return;
	}
	chunk->indices = new_indices;

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
	// chunk_set_block(chunk, 0, 0, 0, BLOCK_GRASS);

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
	free(chunk->blocks);
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
                	Block nb = neighbors[DIR_POS_X] ? neighbors[DIR_POS_X]->blocks[chunk_get_block_index(0, y, z)] : BLOCK_AIR;
                	if (nb == BLOCK_AIR) add_face(chunk, posf, DIR_POS_X, bt);
            	} else if (chunk->blocks[chunk_get_block_index(x+1, y, z)] == BLOCK_AIR) {
                	add_face(chunk, posf, DIR_POS_X, bt);
            	}

            	// -X
            	if (x == 0) {
                	Block nb = neighbors[DIR_NEG_X] ? neighbors[DIR_NEG_X]->blocks[chunk_get_block_index(CHUNK_SIZE-1, y, z)] : BLOCK_AIR;
                	if (nb == BLOCK_AIR) add_face(chunk, posf, DIR_NEG_X, bt);
            	} else if (chunk->blocks[chunk_get_block_index(x-1, y, z)] == BLOCK_AIR) {
                	add_face(chunk, posf, DIR_NEG_X, bt);
            	}

            	// +Y
            	if (y == CHUNK_SIZE - 1) {
                	Block nb = neighbors[DIR_POS_Y] ? neighbors[DIR_POS_Y]->blocks[chunk_get_block_index(x, 0, z)] : BLOCK_AIR;
                	if (nb == BLOCK_AIR) add_face(chunk, posf, DIR_POS_Y, bt);
            	} else if (chunk->blocks[chunk_get_block_index(x, y+1, z)] == BLOCK_AIR) {
                	add_face(chunk, posf, DIR_POS_Y, bt);
            	}

            	// -Y
            	if (y == 0) {
                	Block nb = neighbors[DIR_NEG_Y] ? neighbors[DIR_NEG_Y]->blocks[chunk_get_block_index(x, CHUNK_SIZE-1, z)] : BLOCK_AIR;
                	if (nb == BLOCK_AIR) add_face(chunk, posf, DIR_NEG_Y, bt);
            	} else if (chunk->blocks[chunk_get_block_index(x, y-1, z)] == BLOCK_AIR) {
                	add_face(chunk, posf, DIR_NEG_Y, bt);
            	}

            	// +Z
            	if (z == CHUNK_SIZE - 1) {
                	Block nb = neighbors[DIR_POS_Z] ? neighbors[DIR_POS_Z]->blocks[chunk_get_block_index(x, y, 0)] : BLOCK_AIR;
                	if (nb == BLOCK_AIR) add_face(chunk, posf, DIR_POS_Z, bt);
            	} else if (chunk->blocks[chunk_get_block_index(x, y, z+1)] == BLOCK_AIR) {
                	add_face(chunk, posf, DIR_POS_Z, bt);
            	}

            	// -Z
            	if (z == 0) {
                	Block nb = neighbors[DIR_NEG_Z] ? neighbors[DIR_NEG_Z]->blocks[chunk_get_block_index(x, y, CHUNK_SIZE-1)] : BLOCK_AIR;
                	if (nb == BLOCK_AIR) add_face(chunk, posf, DIR_NEG_Z, bt);
            	} else if (chunk->blocks[chunk_get_block_index(x, y, z-1)] == BLOCK_AIR) {
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

void chunk_rebuild_block(World* world, Chunk* chunk, int cx, int cy, int cz, int x, int y, int z) {
	const int direction_offsets[DIR_COUNT][3] = {
    	{ 1,  0,  0}, // DIR_POS_X
    	{-1,  0,  0}, // DIR_NEG_X
    	{ 0,  1,  0}, // DIR_POS_Y
    	{ 0, -1,  0}, // DIR_NEG_Y
    	{ 0,  0,  1}, // DIR_POS_Z
    	{ 0,  0, -1}  // DIR_NEG_Z
	};

	// Loop over the block and its 6 neighbors
    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            for (int dz = -1; dz <= 1; dz++) {
                int bx = x + dx;
                int by = y + dy;
                int bz = z + dz;

                // Skip out-of-bounds blocks
                if (bx < 0 || bx >= CHUNK_SIZE ||
                    by < 0 || by >= CHUNK_SIZE ||
                    bz < 0 || bz >= CHUNK_SIZE) {
                    continue;
                }

                Block block = chunk_get_block(chunk, bx, by, bz);
                if (block == BLOCK_AIR) continue;

                for (Direction dir = 0; dir < DIR_COUNT; dir++) {
                    int nx = bx + direction_offsets[dir][0];
                    int ny = by + direction_offsets[dir][1];
                    int nz = bz + direction_offsets[dir][2];

                    Block neighbor = world_get_block(world,
                        cx * CHUNK_SIZE + nx,
                        cy * CHUNK_SIZE + ny,
                        cz * CHUNK_SIZE + nz);
					
					vec3 posf = { (float)bx, (float)by, (float)bz };
                    if (neighbor == BLOCK_AIR) {
                        // mesh_add_face(chunk->mesh, bx, by, bz, dir);
						// add_face(chunk, posf, DIR_POS_Z, block);
						printf("add");
                    } else {
						printf("remove");
                        // mesh_remove_face(chunk->mesh, bx, by, bz, dir);
                    }
                }
			}
        }
    }

    // Upload updated mesh to GPU
    // mesh_upload(chunk->mesh);
}

void chunk_draw(const Chunk* chunk, Shader* shader) {
	glBindVertexArray(chunk->vao);
	glDrawElements(GL_TRIANGLES, (GLsizei)chunk->index_count, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}
