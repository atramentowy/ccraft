#include "world.h"
#include "frustum.h"
#include "perlin.h"

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

    return &world->chunks[nx][ny][nz];
}

void world_set_block(World* world, int x, int y, int z, Block block) {
	int chunk_x = floor(x / CHUNK_SIZE);
	int chunk_y = floor(y / CHUNK_SIZE);
	int chunk_z = floor(z / CHUNK_SIZE);

	int block_x = x % CHUNK_SIZE;
	int block_y = y % CHUNK_SIZE;
	int block_z = z % CHUNK_SIZE;

	world->chunks[chunk_x][chunk_y][chunk_z].blocks[chunk_get_block_index(
			block_x, block_y, block_z)] = block;
}

void world_init(World* world) {
    for (int x = 0; x < WORLD_SIZE_X; x++) {
        for (int y = 0; y < WORLD_SIZE_Y; y++) {
            for (int z = 0; z < WORLD_SIZE_Z; z++) {
                chunk_init(&world->chunks[x][y][z]);
            }
        }
    }
	world_generate(world);
}

void world_unload(World* world) {
    for (int x = 0; x < WORLD_SIZE_X; x++) {
        for (int y = 0; y < WORLD_SIZE_Y; y++) {
            for (int z = 0; z < WORLD_SIZE_Z; z++) {
                chunk_unload(&world->chunks[x][y][z]);
            }
        }
    }
}

void world_generate(World* world) {
	int p[512];
	init_perlin(p);

	for (int x = 0; x < WORLD_SIZE_X * CHUNK_SIZE; x++) {
        for (int y = 0; y < WORLD_SIZE_Y * CHUNK_SIZE; y++) {
            for (int z = 0; z < WORLD_SIZE_Z * CHUNK_SIZE; z++) {
				float nx = x * 0.1f;
    			float ny = y * 0.1f;
   				float nz = z * 0.1f;

    			float noise = perlin_noise_3d(nx, ny, nz, p);
    			float density = (noise + 1.0f) * 0.5f; // Normalize to [0, 1]

    			int block = (density > 0.5f) ? 2 : 0;
				world_set_block(world, x, y, z, block);
			}
		}
	}
}

void world_rebuild(World* world) {
    for (int x = 0; x < WORLD_SIZE_X; x++) {
        for (int y = 0; y < WORLD_SIZE_Y; y++) {
            for (int z = 0; z < WORLD_SIZE_Z; z++) {
				if (!&world->chunks[x][y][z].dirty) continue;

        		chunk_rebuild(world, &world->chunks[x][y][z], x, y, z);
            }
        }
    }
}

void world_draw(const RenderContext* ctx, World* world, Shader* shader) {
	shader_use(shader);
	shader_set_mat4(shader, "projection", ctx->projection);
	shader_set_mat4(shader, "view", ctx->view);
	
    for (int x = 0; x < WORLD_SIZE_X; x++) {
        for (int y = 0; y < WORLD_SIZE_Y; y++) {
            for (int z = 0; z < WORLD_SIZE_Z; z++) {
				Chunk* chunk = &world->chunks[x][y][z];

				// update visibility
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
    }
}
