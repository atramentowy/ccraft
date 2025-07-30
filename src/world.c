#include "world.h"

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
        		chunk_rebuild(&world->chunks[x][y][z]);
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

