#include "chunk.h"
#include "world.h"

#include <cglm/cglm.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

void add_face(Chunk* chunk, vec3 pos, int face, BlockType block_type, uint8_t light_level) {
    const float tile_size = 1.0f / 16.0f; // 16x16 tiles
    const float epsilon = 0.001f;

    int tile_x = block_type % 16;
    int tile_y = block_type / 16;

    tile_y = 15 - tile_y;

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
        {
            {0.5f,  0.5f, -0.5f},
            {0.5f,  0.5f,  0.5f},
            {0.5f, -0.5f,  0.5f},
            {0.5f, -0.5f, -0.5f}
        },
        // -X
        {
            {-0.5f,  0.5f,  0.5f},
            {-0.5f,  0.5f, -0.5f},
            {-0.5f, -0.5f, -0.5f},
            {-0.5f, -0.5f,  0.5f}
        },
        // +Y
        {
            { 0.5f, 0.5f, -0.5f},
            {-0.5f, 0.5f, -0.5f},
            {-0.5f, 0.5f,  0.5f},
            { 0.5f, 0.5f,  0.5f}},
        // -Y
        {
            {-0.5f, -0.5f, -0.5f},
            { 0.5f, -0.5f, -0.5f},
            { 0.5f, -0.5f,  0.5f},
            {-0.5f, -0.5f,  0.5f}
        },
        // +Z
        {
            { 0.5f,  0.5f, 0.5f},
            {-0.5f,  0.5f, 0.5f},
            {-0.5f, -0.5f, 0.5f},
            { 0.5f, -0.5f, 0.5f}
        },
        // -Z
        {
            {-0.5f,  0.5f, -0.5f},
            { 0.5f,  0.5f, -0.5f},
            { 0.5f, -0.5f, -0.5f},
            {-0.5f, -0.5f, -0.5f}
        }
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
        glm_vec3_add(v.position, pos, v.position);

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

void chunk_init(Chunk* chunk, int index) {
	chunk->blocks = malloc(MAX_CHUNK_SIZE * sizeof(Block));
	// memset(chunk->blocks, 0, MAX_CHUNK_SIZE * sizeof(Block));
    
    for(int i = 0; i < MAX_CHUNK_SIZE; i++) {
        chunk->blocks[i].type = BLOCK_AIR;
        chunk->blocks[i].light_level = 0;
    }

    chunk->blocks[chunk_get_block_index(0, 1, 0)].type = BLOCK_LIGHT;
    
    lightqueue_init(&chunk->light_queue);
    lightqueue_init(&chunk->border_light_queue);

    // lightqueue_push(&chunk->light_queue, (LightNode){0, 1, 0, 15});
	
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
    chunk->active = true;

    // seed light
    for (int x = 0; x < CHUNK_SIZE; x++) {
        for (int y = 0; y < CHUNK_SIZE; y++) {
            for (int z = 0; z < CHUNK_SIZE; z++) {
                Block* block = &chunk->blocks[chunk_get_block_index(x, y, z)];
                if (block->type == BLOCK_LIGHT) {
                    int ch_x = index % WORLD_SIZE_X;
                    int ch_y = (index / WORLD_SIZE_X) % WORLD_SIZE_Y;
                    int ch_z = index / (WORLD_SIZE_X * WORLD_SIZE_Y);

                    uint8_t emission = block_get_emission(block->type);
                    block->light_level = emission;

                    lightqueue_push(
                        &chunk->light_queue,
                        (LightNode){
                            x + (ch_x * CHUNK_SIZE),
                            y + (ch_y * CHUNK_SIZE),
                            z + (ch_z * CHUNK_SIZE),
                            emission
                        }
                    );
                }
            }
        }
    }
}

void chunk_unload(Chunk* chunk) {
    if (chunk->vao)
        glDeleteVertexArrays(1, &chunk->vao);
    if (chunk->vbo)
        glDeleteBuffers(1, &chunk->vbo);
    if (chunk->ebo)
        glDeleteBuffers(1, &chunk->ebo);

    chunk->vao = 0;
    chunk->vbo = 0;
    chunk->ebo = 0;

    free(chunk->blocks);
    free(chunk->vertices);
    free(chunk->indices);
}

void chunk_update_mesh(World* world, Chunk* chunk, int ch_x, int ch_y, int ch_z) {
    // chunk_update_light(world, chunk, (ivec3){ch_x, ch_y, ch_z});

	free(chunk->vertices);
    free(chunk->indices);
    chunk->vertices = NULL;
    chunk->indices = NULL;
    chunk->vertex_count = 0;
    chunk->index_count = 0;

	Chunk* neighbors[DIR_COUNT] = {0};
	for (Direction d = 0; d < DIR_COUNT; ++d) {
		neighbors[d] = chunk_get_neighbor(world, ch_x, ch_y, ch_z, d);
	}

	for (int x = 0; x < CHUNK_SIZE; ++x) {
   		for (int y = 0; y < CHUNK_SIZE; ++y) {
        	for (int z = 0; z < CHUNK_SIZE; ++z) {
				BlockType bt = chunk->blocks[chunk_get_block_index(x, y, z)].type;
                uint8_t light_level = chunk->blocks[chunk_get_block_index(x, y, z)].light_level;
            	if (bt == BLOCK_AIR) continue;

            	vec3 posf = { (float)x, (float)y, (float)z };

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

void chunk_update_light(World* world, Chunk* chunk, int index) {
    if (!chunk || !chunk->blocks) return;

    int ch_x = index % WORLD_SIZE_X;
    int ch_y = (index / WORLD_SIZE_X) % WORLD_SIZE_Y;
    int ch_z = index / (WORLD_SIZE_X * WORLD_SIZE_Y);

    // Process border light queue
    while (!lightqueue_empty(&chunk->border_light_queue)) {
        LightNode node = lightqueue_pop(&chunk->border_light_queue);
        lightqueue_push(&chunk->light_queue, node);
    }

    // Light propagation
    const int dir_x[6] = { 1, -1,  0,  0,  0,  0 };
    const int dir_y[6] = { 0,  0,  1, -1,  0,  0 };
    const int dir_z[6] = { 0,  0,  0,  0,  1, -1 };

    while (!lightqueue_empty(&chunk->light_queue)) {
        LightNode node = lightqueue_pop(&chunk->light_queue);

        for (int dir = 0; dir < 6; dir++) {
            vec3 neighbor_pos = {
                node.x + dir_x[dir],
                node.y + dir_y[dir],
                node.z + dir_z[dir]
            };

            int ncx = (int)neighbor_pos[0] / CHUNK_SIZE;
            int ncy = (int)neighbor_pos[1] / CHUNK_SIZE;
            int ncz = (int)neighbor_pos[2] / CHUNK_SIZE;

            if (ncx == ch_x && ncy == ch_y && ncz == ch_z) {
                int lx = (int)neighbor_pos[0] % CHUNK_SIZE;
                int ly = (int)neighbor_pos[1] % CHUNK_SIZE;
                int lz = (int)neighbor_pos[2] % CHUNK_SIZE;

                if (lx < 0 || ly < 0 || lz < 0 || lx >= CHUNK_SIZE || ly >= CHUNK_SIZE || lz >= CHUNK_SIZE) continue;

                Block* neighbor = &chunk->blocks[chunk_get_block_index(lx, ly, lz)];

                if (!block_is_transparent(neighbor->type)) {
                    neighbor->light_level = 0;
                    continue;
                }
                
                int new_level = node.light -1;
                if (new_level > 0 && new_level > neighbor->light_level) {
                    neighbor->light_level = new_level;
                    lightqueue_push(
                        &chunk->light_queue, 
                        (LightNode) {neighbor_pos[0], neighbor_pos[1], neighbor_pos[2], new_level}
                    );
                }
            } else {
                Chunk* nb_chunk = &world->chunks[world_get_chunk_index(ncx, ncy, ncz)];

                if (ncx < 0 || ncx >= WORLD_SIZE_X ||
                    ncy < 0 || ncy >= WORLD_SIZE_Y ||
                    ncz < 0 || ncz >= WORLD_SIZE_Z) continue;

                if (!nb_chunk || !nb_chunk->blocks) continue;

                int lx = ((int)neighbor_pos[0]) % CHUNK_SIZE;
                int ly = ((int)neighbor_pos[1]) % CHUNK_SIZE;
                int lz = ((int)neighbor_pos[2]) % CHUNK_SIZE;

                if (lx < 0 || ly < 0 || lz < 0 || lx >= CHUNK_SIZE || ly >= CHUNK_SIZE || lz >= CHUNK_SIZE) continue;

                Block* neighbor = &nb_chunk->blocks[chunk_get_block_index(lx, ly, lz)];
                if (!block_is_transparent(neighbor->type)) continue;

                // propagate
                int new_level = node.light -1;
                if (new_level > 0 && new_level > neighbor->light_level) {
                    neighbor->light_level = new_level;

                    lightqueue_push(
                        &nb_chunk->border_light_queue,
                        (LightNode) {(int)neighbor_pos[0], (int)neighbor_pos[1], (int)neighbor_pos[2], new_level
                    });
                    nb_chunk->active = true;
                }
            }
        }
    }
}

void chunk_draw(const Chunk* chunk, Shader* shader) {
	glBindVertexArray(chunk->vao);
	glDrawElements(GL_TRIANGLES, (GLsizei)chunk->index_count, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

