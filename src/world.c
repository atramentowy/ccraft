#include "world.h"

/*
int chunk_index(int x, int y, int z) {
	return x + WORLD_SIZE_X * (y + WORLD_SIZE_Y * z);
}

void chunk_coords_from_index(int index, int* x, int* y, int* z) {
    *x = index % WORLD_SIZE_X;
    *y = (index / WORLD_SIZE_X) % WORLD_SIZE_Y;
    *z = index / (WORLD_SIZE_X * WORLD_SIZE_Y);
}

Chunk* world_get_chunk_at(World* world, int x, int y, int z) {
    if (x < 0 || x >= WORLD_SIZE_X ||
        y < 0 || y >= WORLD_SIZE_Y ||
        z < 0 || z >= WORLD_SIZE_Z) {
        return NULL;
    }

    int index = 0; // chunk_index(x, y, z);
    return &world->chunks[index];
}
*/

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

void world_init(World* world) {
    for (int x = 0; x < WORLD_SIZE_X; x++) {
        for (int y = 0; y < WORLD_SIZE_Y; y++) {
            for (int z = 0; z < WORLD_SIZE_Z; z++) {
                chunk_init(&world->chunks[x][y][z]);
            }
        }
    }
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

void world_rebuild(World* world) {
    for (int x = 0; x < WORLD_SIZE_X; x++) {
        for (int y = 0; y < WORLD_SIZE_Y; y++) {
            for (int z = 0; z < WORLD_SIZE_Z; z++) {
        		chunk_rebuild(world, &world->chunks[x][y][z], x, y, z);
            }
        }
    }
}

void world_draw(const World* world, Shader* shader) {
    for (int x = 0; x < WORLD_SIZE_X; x++) {
        for (int y = 0; y < WORLD_SIZE_Y; y++) {
            for (int z = 0; z < WORLD_SIZE_Z; z++) {
				mat4 model;
				glm_mat4_identity(model);

				vec3 translation = {
					x * CHUNK_SIZE,
					y * CHUNK_SIZE,
					z * CHUNK_SIZE
				};
				glm_translate(model, translation);

				shader_set_mat4(shader, "model", model);

				chunk_draw(&world->chunks[x][y][z], shader);
            }
        }
    }
}
