#include "world.h"
#include "frustum.h"
#include "perlin.h"

int world_get_chunk_index(int x, int y, int z) {
    if (x < 0 || x >= WORLD_SIZE_X || 
        y < 0 || y >= WORLD_SIZE_Y ||
        z < 0 || z >= WORLD_SIZE_Z) {
        return -1;
    }
    return x * WORLD_SIZE_X * WORLD_SIZE_Y + y * WORLD_SIZE_Z + z;
}

Chunk* world_get_chunk(World* world, int x, int y, int z) {
    if (x < 0 || x >= WORLD_SIZE_X ||
        y < 0 || y >= WORLD_SIZE_Y ||
        z < 0 || z >= WORLD_SIZE_Z) {
        return NULL;
    }
    return &world->chunks[world_get_chunk_index(x, y, z)];
}

void world_set_block(World* world, int x, int y, int z, BlockType block) {
    // check if the coordinates are inside the world
    if (x < 0 || x >= CHUNK_SIZE * WORLD_SIZE_X ||
        y < 0 || y >= CHUNK_SIZE * WORLD_SIZE_Y ||
        z < 0 || z >= CHUNK_SIZE * WORLD_SIZE_Z) {
        return;
    }
    
    int ch_x = x / CHUNK_SIZE;
    int ch_y = y / CHUNK_SIZE;
    int ch_z = z / CHUNK_SIZE;

    int b_x = x % CHUNK_SIZE;
    int b_y = y % CHUNK_SIZE;
    int b_z = z % CHUNK_SIZE;

    Chunk* chunk = &world->chunks[world_get_chunk_index(ch_x, ch_y, ch_z)];
    chunk->blocks[chunk_get_block_index(b_x, b_y, b_z)].type = block;
}

BlockType world_get_block(World* world, int x, int y, int z) {
	// check if the coordinates are inside the world
    if (x < 0 || x >= CHUNK_SIZE * WORLD_SIZE_X ||
        y < 0 || y >= CHUNK_SIZE * WORLD_SIZE_Y ||
        z < 0 || z >= CHUNK_SIZE * WORLD_SIZE_Z) {
        return BLOCK_AIR;
    }
	int ch_x = x / CHUNK_SIZE;
	int ch_y = y / CHUNK_SIZE;
	int ch_z = z / CHUNK_SIZE;

	int b_x = x % CHUNK_SIZE;
	int b_y = y % CHUNK_SIZE;
	int b_z = z % CHUNK_SIZE;
	
	Chunk* chunk = &world->chunks[world_get_chunk_index(ch_x, ch_y, ch_z)];
	return chunk->blocks[chunk_get_block_index(b_x, b_y, b_z)].type;
}

void world_add_block(World* world, int x, int y, int z, BlockType block) {
    world_set_block(world, x, y, z, block);

    // chunk coordinates
    int ch_x = x / CHUNK_SIZE;
    int ch_y = y / CHUNK_SIZE;
    int ch_z = z / CHUNK_SIZE;

    // local block coordinates inside chunk
    int b_x = x % CHUNK_SIZE;
    int b_y = y % CHUNK_SIZE;
    int b_z = z % CHUNK_SIZE;

    // mark neighbors dirty if at chunk boundaries
    // x
    if (b_x == 0 && ch_x > 0) {
        world->chunks[world_get_chunk_index(ch_x - 1, ch_y, ch_z)].dirty_light = true;
    } else if (b_x == CHUNK_SIZE - 1 && ch_x < WORLD_SIZE_X - 1) {
        world->chunks[world_get_chunk_index(ch_x + 1, ch_y, ch_z)].dirty_light = true;
    }

    // y
    if (b_y == 0 && ch_y > 0) {
        world->chunks[world_get_chunk_index(ch_x, ch_y - 1, ch_z)].dirty_light = true;
    } else if (b_y == CHUNK_SIZE - 1 && ch_y < WORLD_SIZE_Y - 1) {
        world->chunks[world_get_chunk_index(ch_x, ch_y + 1, ch_z)].dirty_light = true;
    }

    // z
    if (b_z == 0 && ch_z > 0) {
        world->chunks[world_get_chunk_index(ch_x, ch_y, ch_z - 1)].dirty_light = true;
    } else if (b_z == CHUNK_SIZE - 1 && ch_z < WORLD_SIZE_Z - 1) {
        world->chunks[world_get_chunk_index(ch_x, ch_y, ch_z + 1)].dirty_light = true;
    }

    // mark current chunk as dirty
    world->chunks[world_get_chunk_index(ch_x, ch_y, ch_z)].dirty_light = true;
}

void world_remove_block(World* world, int x, int y, int z) {
    world_set_block(world, x, y, z, BLOCK_AIR);

    // chunk coordinates
    int ch_x = x / CHUNK_SIZE;
    int ch_y = y / CHUNK_SIZE;
    int ch_z = z / CHUNK_SIZE;

    // local block coordinates inside chunk
    int b_x = x % CHUNK_SIZE;
    int b_y = y % CHUNK_SIZE;
    int b_z = z % CHUNK_SIZE;

    // mark neighbors dirty if at chunk boundaries
    // x
    if (b_x == 0 && ch_x > 0) {
        world->chunks[world_get_chunk_index(ch_x - 1, ch_y, ch_z)].dirty_light = true;
    } else if (b_x == CHUNK_SIZE - 1 && ch_x < WORLD_SIZE_X - 1) {
        world->chunks[world_get_chunk_index(ch_x + 1, ch_y, ch_z)].dirty_light = true;
    }

    // y
    if (b_y == 0 && ch_y > 0) {
        world->chunks[world_get_chunk_index(ch_x, ch_y - 1, ch_z)].dirty_light = true;
    } else if (b_y == CHUNK_SIZE - 1 && ch_y < WORLD_SIZE_Y - 1) {
        world->chunks[world_get_chunk_index(ch_x, ch_y + 1, ch_z)].dirty_light = true;
    }

    // z
    if (b_z == 0 && ch_z > 0) {
        world->chunks[world_get_chunk_index(ch_x, ch_y, ch_z - 1)].dirty_light = true;
    } else if (b_z == CHUNK_SIZE - 1 && ch_z < WORLD_SIZE_Z - 1) {
        world->chunks[world_get_chunk_index(ch_x, ch_y, ch_z + 1)].dirty_light = true;
    }

    // mark current chunk as dirty
    world->chunks[world_get_chunk_index(ch_x, ch_y, ch_z)].dirty_light = true;
}

void world_init(World* world) {
    world->chunks = malloc(MAX_WORLD_SIZE * sizeof(Chunk));
    memset(world->chunks, 0, MAX_WORLD_SIZE * sizeof(Chunk));

    for(int i = 0; i < MAX_WORLD_SIZE; i++) {
        chunk_init(&world->chunks[i], i);
    }
	world_generate(world);
}

void world_unload(World* world) {
    if (!world || !world->chunks) return;

    int chunk_count = MAX_WORLD_SIZE;

    for (int i = 0; i < chunk_count; i++) {
        chunk_unload(&world->chunks[i]);
    }

    free(world->chunks);
    world->chunks = NULL;
}

void world_generate(World* world) {
	for (int x = 0; x < WORLD_SIZE_X * CHUNK_SIZE; x++) {
        for (int y = 0; y < WORLD_SIZE_Y * CHUNK_SIZE; y++) {
            for (int z = 0; z < WORLD_SIZE_Z * CHUNK_SIZE; z++) {
				world_set_block(world, x, 0, z, BLOCK_GRASS);
			}
		}
	}

	// world_set_block(world, 0, 1, 0, BLOCK_LIGHT);
	// world_set_block(world, 0, 2, 0, BLOCK_GRASS);
	// world_set_block(world, 6, 2, 6, BLOCK_GRASS);

	/*
	int p[512];
	init_perlin(p);

	const int MAX_HEIGHT = 2;

	for (int x = 0; x < WORLD_SIZE_X * CHUNK_SIZE; x++) {
        for (int y = 0; y < WORLD_SIZE_Y * CHUNK_SIZE; y++) {
            for (int z = 0; z < WORLD_SIZE_Z * CHUNK_SIZE; z++) {
                
                float nx = x * 0.01f;
                float nz = z * 0.01f;
                float height = fbm(nx, 0.0f, nz, p, 6);
                int terrain_height = (int)((height + 1.0f) * 0.5f * MAX_HEIGHT);

                int block;

                if (y < terrain_height - 3) {
                    block = BLOCK_STONE;
                } else if (y < terrain_height) {
                    block = BLOCK_STONE;
                } else if (y == terrain_height) {
                    block = BLOCK_GRASS;
                } else {
                    block = BLOCK_AIR;
                }

                float cave_density = perlin_noise_3d(x * 0.1f, y * 0.1f, z * 0.1f, p);
                float cave_value = (cave_density + 1.0f) * 0.5f;

                if (cave_value > 0.6f && y < terrain_height) {
                    block = BLOCK_AIR;
                }

                world_set_block(world, x, y, z, block);
            }
        }
    }
	*/
}

void world_update_mesh(World* world) {
    for(int i = 0; i < MAX_WORLD_SIZE; i++) {
        if(!world->chunks[i].dirty) continue;

        chunk_update_mesh(world, &world->chunks[i], i);
    }
}

void world_update_light(World* world) {
    bool any_dirty = true; // for first iteration
    while(any_dirty) {
        any_dirty = false;

        for (int i = 0; i < MAX_WORLD_SIZE; i++) {
            Chunk* chunk = &world->chunks[i];
            if (chunk && chunk->dirty_light) {
                chunk->dirty_light = false;
                chunk_update_light(world, chunk, i);
                chunk->dirty = true;
                any_dirty = true;
            }
        }
    }
}

void world_draw(const RenderContext* ctx, World* world, Shader* shader) {
	shader_use(shader);
	shader_set_mat4(shader, "projection", ctx->projection);
	shader_set_mat4(shader, "view", ctx->view);
	
    for(int i = 0; i < MAX_WORLD_SIZE; i++) {
        Chunk* chunk = &world->chunks[i];
        
        int x = i / (WORLD_SIZE_Y * WORLD_SIZE_Z);
        int y = (i / WORLD_SIZE_Z) % WORLD_SIZE_Y;
        int z = i % WORLD_SIZE_Z;
        
        // check if chunk is in frustum
		chunk->visible = chunk_in_frustum(&ctx->frustum, x, y, z);
        if(!chunk->visible) continue;

        // set model matrix
       	mat4 model;
		glm_mat4_identity(model);
		vec3 translation = {
			x * CHUNK_SIZE,
			y * CHUNK_SIZE,
			z * CHUNK_SIZE
		};
		glm_translate(model, translation);
		shader_set_mat4(shader, "model", model);

		// draw chunk
		chunk_draw(chunk, shader);
    }
}

