#include "world.h"
#include "frustum.h"
#include "perlin.h"

int world_get_chunk_index(int x, int y, int z) {
    if (x < 0 || x >= WORLD_SIZE_X || y < 0 || y >= WORLD_SIZE_Y || z < 0 || z >= WORLD_SIZE_Z) {
        return -1;
    }
    return x * WORLD_SIZE_X * WORLD_SIZE_Y + y * WORLD_SIZE_Z + z;
}

Chunk* chunk_get_neighbor(World* world, int x, int y, int z, Direction dir) {
	int offset_x(Direction dir) {
        switch (dir) {
            case DIR_POS_X: return 1;
            case DIR_NEG_X: return -1;
            default: return 0;
        }
    }

    int offset_y(Direction dir) {
        switch (dir) {
            case DIR_POS_Y: return 1;
            case DIR_NEG_Y: return -1;
            default: return 0;
        }
    }

    int offset_z(Direction dir) {
        switch (dir) {
            case DIR_POS_Z: return 1;
            case DIR_NEG_Z: return -1;
            default: return 0;
        }
    }

    int nx = x + offset_x(dir);
    int ny = y + offset_y(dir);
    int nz = z + offset_z(dir);

    if (nx < 0 || nx >= WORLD_SIZE_X ||
        ny < 0 || ny >= WORLD_SIZE_Y ||
        nz < 0 || nz >= WORLD_SIZE_Z) {
        return NULL;
    }

    return &world->chunks[world_get_chunk_index(nx, ny, nz)];
}

BlockType world_get_block(World* world, int x, int y, int z) {
	// Check if the coordinates are inside the world
    if (x < 0 || x >= CHUNK_SIZE * WORLD_SIZE_X ||
        y < 0 || y >= CHUNK_SIZE * WORLD_SIZE_Y ||
        z < 0 || z >= CHUNK_SIZE * WORLD_SIZE_Z) {
        return BLOCK_AIR;
    }
	int chunk_x = x / CHUNK_SIZE;
	int chunk_y = y / CHUNK_SIZE;
	int chunk_z = z / CHUNK_SIZE;

	int block_x = x % CHUNK_SIZE;
	int block_y = y % CHUNK_SIZE;
	int block_z = z % CHUNK_SIZE;
	
	Chunk* chunk = &world->chunks[world_get_chunk_index(chunk_x, chunk_y, chunk_z)];
	return chunk->blocks[chunk_get_block_index(block_x, block_y, block_z)].type;
}

void world_set_block(World* world, int x, int y, int z, BlockType block) { // idk if it should return value
    // Check if the coordinates are inside the world
    if (x < 0 || x >= CHUNK_SIZE * WORLD_SIZE_X ||
        y < 0 || y >= CHUNK_SIZE * WORLD_SIZE_Y ||
        z < 0 || z >= CHUNK_SIZE * WORLD_SIZE_Z) {
        return;
    }

    int chunk_x = x / CHUNK_SIZE;
    int chunk_y = y / CHUNK_SIZE;
    int chunk_z = z / CHUNK_SIZE;

    int block_x = x % CHUNK_SIZE;
    int block_y = y % CHUNK_SIZE;
    int block_z = z % CHUNK_SIZE;

    Chunk* chunk = &world->chunks[world_get_chunk_index(chunk_x, chunk_y, chunk_z)];
    chunk->blocks[chunk_get_block_index(block_x, block_y, block_z)].type = block;
}

void world_init(World* world) {
    world->chunks = malloc(WORLD_SIZE_X * WORLD_SIZE_Y * WORLD_SIZE_Z * sizeof(Chunk));
    memset(world->chunks, 0, WORLD_SIZE_X * WORLD_SIZE_Y * WORLD_SIZE_Z * sizeof(Chunk));

    for(int i = 0; i < WORLD_SIZE_X * WORLD_SIZE_Y * WORLD_SIZE_Z; i++) {
        chunk_init(&world->chunks[i]);
    }
	world_generate(world);
}

void world_unload(World* world) {
    if (!world || !world->chunks) return;

    int chunk_count = WORLD_SIZE_X * WORLD_SIZE_Y * WORLD_SIZE_Z;

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

	world_set_block(world, 0, 1, 0, BLOCK_LIGHT);
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
    for(int i = 0; i < WORLD_SIZE_X * WORLD_SIZE_Y * WORLD_SIZE_Z; i++) {
        if(!world->chunks[i].dirty) continue;

        int x = i / (WORLD_SIZE_Y * WORLD_SIZE_Z);
        int y = (i / WORLD_SIZE_Z) % WORLD_SIZE_Y;
        int z = i % WORLD_SIZE_Z;

        chunk_update_mesh(world, &world->chunks[i], x, y, z);
    }
}

void world_update_light(World* world) {
    for(int i = 0; i < WORLD_SIZE_X * WORLD_SIZE_Y * WORLD_SIZE_Z; i++) {
        chunk_update_light(world, &world->chunks[i]);
    }
}

void world_draw(const RenderContext* ctx, World* world, Shader* shader) {
	shader_use(shader);
	shader_set_mat4(shader, "projection", ctx->projection);
	shader_set_mat4(shader, "view", ctx->view);
	
    for(int i = 0; i < WORLD_SIZE_X * WORLD_SIZE_Y * WORLD_SIZE_Z; i++) {
        Chunk* chunk = &world->chunks[i];
        
        int x = i / (WORLD_SIZE_Y * WORLD_SIZE_Z);
        int y = (i / WORLD_SIZE_Z) % WORLD_SIZE_Y;
        int z = i % WORLD_SIZE_Z;
        
        // rebuild mesh if dirty
        if(chunk->dirty) chunk_update_mesh(world, chunk, x, y, z);
        
        // check if in frustum
		chunk->visible = chunk_in_frustum(&ctx->frustum, x, y, z); // check in frustum
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

