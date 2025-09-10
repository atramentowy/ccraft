#include "player.h"
#include "game.h"

void player_init(Player* player) {	
	vec3 position = {0.0f, 0.0f, 0.0f};
	vec3 up = {0.0f, 1.0f, 0.0f};
	camera_init(&player->camera, position, up, CAMERA_YAW, CAMERA_PITCH);

	player->selected_block = BLOCK_GRASS;
}

void player_update(Player* player, Game* game) {
	float aspect = (float)game->window_width / (float)game->window_height;

	mat4 projection;
	glm_perspective(
		glm_rad(game->player.camera.zoom),
        aspect,
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
		aspect, 
		game->player.camera.fov_rad, 
		game->player.camera.near_plane, 
		game->player.camera.far_plane
	);
	game->ctx.frustum = frustum;
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
