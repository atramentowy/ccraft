#include "player.h"
#include "game.h"

#define CAMERA_OFFSET 0.5f

void player_init(Player* player) {
	vec3 collision_box = {0.5f, 2.0f, 0.5f};
	vec3 spawn_position = {2.0f, 6.0f, 6.0f};
	vec3 up = {0.0f, 1.0f, 0.0f};

	entity_init(&player->entity, spawn_position, 10.0f, collision_box[0], collision_box[1], collision_box[2]);
	vec3 camera_spawn_position;
	glm_vec3_copy(spawn_position, camera_spawn_position);
	camera_spawn_position[1] += CAMERA_OFFSET;
	camera_init(&player->camera, camera_spawn_position, up, CAMERA_YAW, CAMERA_PITCH);

	player->selected_block = BLOCK_GRASS;
	glm_vec3_copy(player->entity.position, player->entity.prev_position);
}

void player_update(Player* player, Game* game) {
	mat4 projection;
	glm_perspective(
		glm_rad(game->player.camera.zoom),
        game->aspect,
        game->player.camera.near_plane,
		game->player.camera.far_plane, projection
	);
	// shader_set_mat4(&myShader, "projection", projection);
	memcpy(game->ctx.projection, projection, sizeof(mat4));

	mat4 view;
	camera_get_view_matrix(&game->player.camera, view);
	// shader_set_mat4(&myShader, "view", view);
	memcpy(game->ctx.view, view, sizeof(mat4));
		
	Frustum frustum = create_frustum_from_camera(
		&game->player.camera, 
		game->aspect, 
		player->camera.fov_rad, 
		player->camera.near_plane, 
		player->camera.far_plane
	);
	game->ctx.frustum = frustum;

	// Update camera position directly from player
	glm_vec3_copy(player->entity.position, player->camera.position);
	player->camera.position[1] += CAMERA_OFFSET;

	// Update previous position for next frame
	glm_vec3_copy(player->entity.position, player->entity.prev_position);
}

void player_set_block(Game* game, Block block) {
	vec3 origin;
	glm_vec3_copy(game->player.camera.position, origin);

	int x = (int)roundf(origin[0]);
	int y = (int)roundf(origin[1]);
	int z = (int)roundf(origin[2]);

	if (world_get_block(&game->world, x, y, z) != block) {
		world_set_block(&game->world, x, y, z, block);
		world_rebuild(&game->world);
	}
	// printf("Placing at (%d, %d, %d)\n", x, y, z);
	// printf("Current block: %d\n", world_get_block(world, x, y, z));
}

void player_place_block(Game* game) {
	player_set_block(game, game->player.selected_block);
}

void player_destroy_block(Game* game) {
	player_set_block(game, BLOCK_AIR);
}

/*
int raycast(World* world, float ox, float oy, float oz, float dx, float dy, float dz,
            int* out_x, int* out_y, int* out_z, float max_distance) {
    const float step = 0.05f;

    for (float t = 0.0f; t < max_distance; t += step) {
        int x = (int)floorf(ox + dx * t);
        int y = (int)floorf(oy + dy * t);
        int z = (int)floorf(oz + dz * t);

        if (x < 0 || x >= WORLD_SIZE_X ||
            y < 0 || y >= WORLD_SIZE_Y ||
            z < 0 || z >= WORLD_SIZE_Z)
            continue;

        if (world_get_block(world, x, y, z) != BLOCK_AIR) {
            int px = x - (dx > 0 ? 1 : (dx < 0 ? -1 : 0));
            int py = y - (dy > 0 ? 1 : (dy < 0 ? -1 : 0));
            int pz = z - (dz > 0 ? 1 : (dz < 0 ? -1 : 0));

            if (px >= 0 && px < WORLD_SIZE_X &&
                py >= 0 && py < WORLD_SIZE_Y &&
                pz >= 0 && pz < WORLD_SIZE_Z) {
                *out_x = px;
                *out_y = py;
                *out_z = pz;
                return 1;
            } else {
                return 0; // Hit block, but can't place
            }
        }
    }
    return 0;
}
*/

