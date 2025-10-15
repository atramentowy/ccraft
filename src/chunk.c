#include "chunk.h"
#include "world.h"

#include <cglm/cglm.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

void add_face(Chunk* chunk, vec3 block_pos, int face, BlockType block_type, uint8_t light_level) {
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

        v.light = light_level / 15.0f; // normalize to 0.0 - 1.0

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
    if (x < 0 || x >= CHUNK_SIZE ||
        y < 0 || y >= CHUNK_SIZE ||
        z < 0 || z >= CHUNK_SIZE) {
        return -1;
    }
    return x * CHUNK_SIZE * CHUNK_SIZE + y * CHUNK_SIZE + z;
}

BlockType chunk_get_block(Chunk* chunk, int x, int y, int z) {
	if (x < 0 || x >= CHUNK_SIZE ||
		y < 0 || y >= CHUNK_SIZE ||
		z < 0 || z >= CHUNK_SIZE) {
		return BLOCK_AIR;
	}
	return chunk->blocks[chunk_get_block_index(x, y, z)].type;
}

void chunk_set_block(Chunk* chunk, int x, int y, int z, BlockType block) {
	if (x < 0 || x >= CHUNK_SIZE ||
        y < 0 || y >= CHUNK_SIZE ||
        z < 0 || z >= CHUNK_SIZE) {
        return;
    }
    chunk->blocks[chunk_get_block_index(x, y, z)].type = block;
}

void chunk_init(Chunk* chunk) {
	chunk->blocks = malloc(CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * sizeof(Block));
	// memset(chunk->blocks, 0, CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * sizeof(Block));
    for(int i = 0; i < CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE; i++) {
        chunk->blocks[i].type = BLOCK_AIR;
        chunk->blocks[i].light_level = 0;
    }

	// chunk->blocks[chunk_get_block_index(0, 0, 0)] = BLOCK_GRASS;

	chunk->vertices = NULL;
	chunk->indices = NULL;
	chunk->vertex_count = 0;
	chunk->index_count = 0;

    chunk->q_front = 0;
    chunk->q_back = 0;
	
	chunk->vao = chunk->vbo = chunk->ebo = 0;
	glGenVertexArrays(1, &chunk->vao);
    glGenBuffers(1, &chunk->vbo);
	glGenBuffers(1, &chunk->ebo);

	chunk->dirty = true;
	chunk->visible = false;
}

void chunk_unload(Chunk* chunk) {
    // Free dynamically allocated block data
    // free(chunk->blocks);

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

void chunk_update_mesh(World* world, Chunk* chunk, int cx, int cy, int cz) {
    chunk_update_light(world, chunk);

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
				BlockType bt = chunk->blocks[chunk_get_block_index(x, y, z)].type;
                uint8_t light_level = chunk->blocks[chunk_get_block_index(x, y, z)].light_level;
            	if (bt == BLOCK_AIR) continue;

            	vec3 posf = { (float)x, (float)y, (float)z };
				/*
				// +X
           		if (x == CHUNK_SIZE - 1) {
                	BlockType nb = neighbors[DIR_POS_X] 
                        ? neighbors[DIR_POS_X]->blocks[chunk_get_block_index(0, y, z)].type : 0;
                	if(nb == BLOCK_AIR || nb != bt)
                        add_face(chunk, posf, DIR_POS_X, bt, light_level);
            	} else {
                    BlockType nb = chunk->blocks[chunk_get_block_index(x+1, y, z)].type;
                    if(nb == BLOCK_AIR || nb != bt)
                        add_face(chunk, posf, DIR_POS_X, bt, light_level);
            	}

            	// -X
            	if (x == 0) {
                	BlockType nb = neighbors[DIR_NEG_X] 
                        ? neighbors[DIR_NEG_X]->blocks[chunk_get_block_index(CHUNK_SIZE-1, y, z)].type : 0;
                	if (nb == BLOCK_AIR || nb != bt)
                        add_face(chunk, posf, DIR_NEG_X, bt, light_level);
            	} else {
                    BlockType nb = chunk->blocks[chunk_get_block_index(x-1, y, z)].type;
                    if(nb == BLOCK_AIR || nb != bt)
                        add_face(chunk, posf, DIR_NEG_X, bt, light_level);
            	}

            	// +Y
            	if (y == CHUNK_SIZE - 1) {
                	BlockType nb = neighbors[DIR_POS_Y] 
                        ? neighbors[DIR_POS_Y]->blocks[chunk_get_block_index(x, 0, z)].type : 0;
                	if (nb == BLOCK_AIR || nb != bt)
                        add_face(chunk, posf, DIR_POS_Y, bt, light_level);
            	} else {
                    BlockType nb = chunk->blocks[chunk_get_block_index(x, y+1, z)].type;
                    if(nb == BLOCK_AIR || nb != bt)
                        add_face(chunk, posf, DIR_POS_Y, bt, light_level);
            	}

            	// -Y
            	if (y == 0) {
                	BlockType nb = neighbors[DIR_NEG_Y] 
                        ? neighbors[DIR_NEG_Y]->blocks[chunk_get_block_index(x, CHUNK_SIZE-1, z)].type : 0;
                	if (nb == BLOCK_AIR || nb != bt)
                        add_face(chunk, posf, DIR_NEG_Y, bt, light_level);
            	} else {
                    BlockType nb = chunk->blocks[chunk_get_block_index(x, y-1, z)].type;
                    if(nb == BLOCK_AIR || nb != bt)
                        add_face(chunk, posf, DIR_NEG_Y, bt, light_level);
            	}

            	// +Z
            	if (z == CHUNK_SIZE - 1) {
                	BlockType nb = neighbors[DIR_POS_Z] 
                        ? neighbors[DIR_POS_Z]->blocks[chunk_get_block_index(x, y, 0)].type : 0;
                	if (nb == BLOCK_AIR || nb != bt)
                        add_face(chunk, posf, DIR_POS_Z, bt, light_level);
            	} else {
                    BlockType nb = chunk->blocks[chunk_get_block_index(x, y, z+1)].type;
                	if(nb == BLOCK_AIR || nb != bt)
                        add_face(chunk, posf, DIR_POS_Z, bt, light_level);
            	}

            	// -Z
            	if (z == 0) {
                	BlockType nb = neighbors[DIR_NEG_Z] 
                        ? neighbors[DIR_NEG_Z]->blocks[chunk_get_block_index(x, y, CHUNK_SIZE-1)].type : 0;
                	if (nb == BLOCK_AIR || nb != bt)
                        add_face(chunk, posf, DIR_NEG_Z, bt, light_level);
            	} else {
                    BlockType nb = chunk->blocks[chunk_get_block_index(x, y, z-1)].type;
                    if(nb == BLOCK_AIR || nb != bt)
                        add_face(chunk, posf, DIR_NEG_Z, bt, light_level);
            	}*/

// +X
if (x == CHUNK_SIZE - 1) {
    Block *neighbor = neighbors[DIR_POS_X]
        ? &neighbors[DIR_POS_X]->blocks[chunk_get_block_index(0, y, z)]
        : NULL;
    BlockType nb = neighbor ? neighbor->type : BLOCK_AIR;
    uint8_t neighbor_light = neighbor ? neighbor->light_level : 0;

    if (nb == BLOCK_AIR || nb != bt)
        add_face(chunk, posf, DIR_POS_X, bt, neighbor_light);
} else {
    Block *neighbor = &chunk->blocks[chunk_get_block_index(x + 1, y, z)];
    BlockType nb = neighbor->type;
    uint8_t neighbor_light = neighbor->light_level;

    if (nb == BLOCK_AIR || nb != bt)
        add_face(chunk, posf, DIR_POS_X, bt, neighbor_light);
}

// -X
if (x == 0) {
    Block *neighbor = neighbors[DIR_NEG_X]
        ? &neighbors[DIR_NEG_X]->blocks[chunk_get_block_index(CHUNK_SIZE - 1, y, z)]
        : NULL;
    BlockType nb = neighbor ? neighbor->type : BLOCK_AIR;
    uint8_t neighbor_light = neighbor ? neighbor->light_level : 0;

    if (nb == BLOCK_AIR || nb != bt)
        add_face(chunk, posf, DIR_NEG_X, bt, neighbor_light);
} else {
    Block *neighbor = &chunk->blocks[chunk_get_block_index(x - 1, y, z)];
    BlockType nb = neighbor->type;
    uint8_t neighbor_light = neighbor->light_level;

    if (nb == BLOCK_AIR || nb != bt)
        add_face(chunk, posf, DIR_NEG_X, bt, neighbor_light);
}

// +Y
if (y == CHUNK_SIZE - 1) {
    Block *neighbor = neighbors[DIR_POS_Y]
        ? &neighbors[DIR_POS_Y]->blocks[chunk_get_block_index(x, 0, z)]
        : NULL;
    BlockType nb = neighbor ? neighbor->type : BLOCK_AIR;
    uint8_t neighbor_light = neighbor ? neighbor->light_level : 0;

    if (nb == BLOCK_AIR || nb != bt)
        add_face(chunk, posf, DIR_POS_Y, bt, neighbor_light);
} else {
    Block *neighbor = &chunk->blocks[chunk_get_block_index(x, y + 1, z)];
    BlockType nb = neighbor->type;
    uint8_t neighbor_light = neighbor->light_level;

    if (nb == BLOCK_AIR || nb != bt)
        add_face(chunk, posf, DIR_POS_Y, bt, neighbor_light);
}

// -Y
if (y == 0) {
    Block *neighbor = neighbors[DIR_NEG_Y]
        ? &neighbors[DIR_NEG_Y]->blocks[chunk_get_block_index(x, CHUNK_SIZE - 1, z)]
        : NULL;
    BlockType nb = neighbor ? neighbor->type : BLOCK_AIR;
    uint8_t neighbor_light = neighbor ? neighbor->light_level : 0;

    if (nb == BLOCK_AIR || nb != bt)
        add_face(chunk, posf, DIR_NEG_Y, bt, neighbor_light);
} else {
    Block *neighbor = &chunk->blocks[chunk_get_block_index(x, y - 1, z)];
    BlockType nb = neighbor->type;
    uint8_t neighbor_light = neighbor->light_level;

    if (nb == BLOCK_AIR || nb != bt)
        add_face(chunk, posf, DIR_NEG_Y, bt, neighbor_light);
}

// +Z
if (z == CHUNK_SIZE - 1) {
    Block *neighbor = neighbors[DIR_POS_Z]
        ? &neighbors[DIR_POS_Z]->blocks[chunk_get_block_index(x, y, 0)]
        : NULL;
    BlockType nb = neighbor ? neighbor->type : BLOCK_AIR;
    uint8_t neighbor_light = neighbor ? neighbor->light_level : 0;

    if (nb == BLOCK_AIR || nb != bt)
        add_face(chunk, posf, DIR_POS_Z, bt, neighbor_light);
} else {
    Block *neighbor = &chunk->blocks[chunk_get_block_index(x, y, z + 1)];
    BlockType nb = neighbor->type;
    uint8_t neighbor_light = neighbor->light_level;

    if (nb == BLOCK_AIR || nb != bt)
        add_face(chunk, posf, DIR_POS_Z, bt, neighbor_light);
}

// -Z
if (z == 0) {
    Block *neighbor = neighbors[DIR_NEG_Z]
        ? &neighbors[DIR_NEG_Z]->blocks[chunk_get_block_index(x, y, CHUNK_SIZE - 1)]
        : NULL;
    BlockType nb = neighbor ? neighbor->type : BLOCK_AIR;
    uint8_t neighbor_light = neighbor ? neighbor->light_level : 0;

    if (nb == BLOCK_AIR || nb != bt)
        add_face(chunk, posf, DIR_NEG_Z, bt, neighbor_light);
} else {
    Block *neighbor = &chunk->blocks[chunk_get_block_index(x, y, z - 1)];
    BlockType nb = neighbor->type;
    uint8_t neighbor_light = neighbor->light_level;

    if (nb == BLOCK_AIR || nb != bt)
        add_face(chunk, posf, DIR_NEG_Z, bt, neighbor_light);
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
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(vec3) + sizeof(vec2)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunk->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * chunk->index_count, chunk->indices, GL_DYNAMIC_DRAW);

    glBindVertexArray(0);

	chunk->dirty = false;
}

void enqueue(Chunk* chunk, LightNode node) {
    int next_back = (chunk->q_back + 1) % MAX_QUEUE;
    if (next_back != chunk->q_front) {
        chunk->queue[chunk->q_back] = node;
        chunk->q_back = next_back;
    } else {
        fprintf(stderr, "Light queue overflow in chunk!\n");
    }
}

bool queue_empty(Chunk* chunk) {
    return chunk->q_front == chunk->q_back;
}

LightNode dequeue(Chunk* chunk) {
    LightNode node = chunk->queue[chunk->q_front];
    chunk->q_front = (chunk->q_front + 1) % MAX_QUEUE;
    return node;
}

bool in_bounds(int x, int y, int z) {
    return (x >= 0 && x < CHUNK_SIZE &&
            y >= 0 && y < CHUNK_SIZE &&
            z >= 0 && z < CHUNK_SIZE);
}

void chunk_update_light(World* world, Chunk* chunk) {
    chunk->q_front = 0;
    chunk->q_back = 0;

    const int dx[DIR_COUNT] = { 1, -1,  0,  0,  0,  0 };
    const int dy[DIR_COUNT] = { 0,  0,  1, -1,  0,  0 };
    const int dz[DIR_COUNT] = { 0,  0,  0,  0,  1, -1 };

    // Reset all light and enqueue light sources
    for (int x = CHUNK_SIZE - 1; x >= 0; x--) {
        for (int y = CHUNK_SIZE - 1; y >= 0; y--) {
            for (int z = CHUNK_SIZE - 1; z >= 0; z--) {

                Block* block = &chunk->blocks[chunk_get_block_index(x, y, z)];
                block->light_level = 0;

                if (block->type == BLOCK_LIGHT) {
                    uint8_t emission = block_get_emission(block->type);
                    block->light_level = emission;
                    // block->light_level = 15;
                    enqueue(chunk, (LightNode){x, y, z, emission});
                }
            }
        }
    }

    // Light propagation
    while (!queue_empty(chunk)) {
        LightNode node = dequeue(chunk);

        for (Direction dir = 0; dir < DIR_COUNT; dir++) {
            int nx = node.x + dx[dir];
            int ny = node.y + dy[dir];
            int nz = node.z + dz[dir];

            if (!in_bounds(nx, ny, nz)) continue;

            Block* neighbor = &chunk->blocks[chunk_get_block_index(nx, ny, nz)];

            if (!block_is_transparent(neighbor->type)) {
                neighbor->light_level = 0;
                continue; // stop light at solid blocks
            }

            if (neighbor->light_level < node.light - 1 && node.light > 1) {
                neighbor->light_level = node.light - 1;
                enqueue(chunk, (LightNode){nx, ny, nz, neighbor->light_level});
            }
        }
    }
}

void chunk_draw(const Chunk* chunk, Shader* shader) {
	glBindVertexArray(chunk->vao);
	glDrawElements(GL_TRIANGLES, (GLsizei)chunk->index_count, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}
