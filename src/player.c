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

bool raycast_voxels(World* world, vec3 origin, vec3 direction, float max_distance, 
		Block* out_block, ivec3* out_coord, ivec3* out_normal) {
	float LARGE = 1e30f; // A safe large number

	// ray origin -> shift by 0.5 to match voxel-center-based coordinates
	int x = (int)floorf(origin[0] + 0.5f);
	int y = (int)floorf(origin[1] + 0.5f);
	int z = (int)floorf(origin[2] + 0.5f);

	// ray step direction
	int step_x = (direction[0] > 0.0f) ? 1 : -1;
	int step_y = (direction[1] > 0.0f) ? 1 : -1;
	int step_z = (direction[2] > 0.0f) ? 1 : -1;

	// position of voxel center
	float voxel_x = (float)x;
	float voxel_y = (float)y;
	float voxel_z = (float)z;

	// distance to next voxel face (aligned with block face boundaries)
	float next_boundary_x = voxel_x + (step_x * 0.5f);
	float next_boundary_y = voxel_y + (step_y * 0.5f);
	float next_boundary_z = voxel_z + (step_z * 0.5f);

	// t_max: distance to first boundary
	float t_max_x = (direction[0] != 0.0f) ? (next_boundary_x - origin[0]) / direction[0] : LARGE;
	float t_max_y = (direction[1] != 0.0f) ? (next_boundary_y - origin[1]) / direction[1] : LARGE;
	float t_max_z = (direction[2] != 0.0f) ? (next_boundary_z - origin[2]) / direction[2] : LARGE;

	// t_delta: distance to cross one voxel
	float t_delta_x = (direction[0] != 0.0f) ? fabsf(1.0f / direction[0]) : LARGE;
	float t_delta_y = (direction[1] != 0.0f) ? fabsf(1.0f / direction[1]) : LARGE;
	float t_delta_z = (direction[2] != 0.0f) ? fabsf(1.0f / direction[2]) : LARGE;

	// calculate
	bool first_step = true;
	float t = 0.0f;

	while(t <= max_distance) {
		if(!first_step) {
			Block block = world_get_block(world, x, y, z);
			if(block != BLOCK_AIR) { // if block is solid
				printf("Ray at voxel (%d, %d, %d), t = %.2f\n", x, y, z, t);

printf("Hit voxel: (%d, %d, %d)\n", x, y, z);
printf("Block bounds: [%.1f, %.1f] x [%.1f, %.1f] x [%.1f, %.1f]\n",
       x - 0.5f, x + 0.5f,
       y - 0.5f, y + 0.5f,
       z - 0.5f, z + 0.5f);

				(*out_coord)[0] = x;
				(*out_coord)[1] = y;
				(*out_coord)[2] = z;

				*out_block = block;
				return true;
			}
		} else {
			first_step = false;
		}


		if (t_max_x < t_max_y) {
    		if (t_max_x < t_max_z) {
        		x += step_x;
        		t = t_max_x;
        		t_max_x += t_delta_x;

				(*out_normal)[0] = -step_x;
				(*out_normal)[1] = 0;
				(*out_normal)[2] = 0;
    		} else {
        		z += step_z;
        		t = t_max_z;
        		t_max_z += t_delta_z;

				(*out_normal)[0] = 0;
				(*out_normal)[1] = 0;
				(*out_normal)[2] = -step_z;
    		}
		} else {
    		if (t_max_y < t_max_z) {
    			y += step_y;
        		t = t_max_y;
       			t_max_y += t_delta_y;

				(*out_normal)[0] = 0;
				(*out_normal)[1] = -step_y;
				(*out_normal)[2] = 0;
    		} else {
       			z += step_z;
        		t = t_max_z;
        		t_max_z += t_delta_z;

				(*out_normal)[0] = 0;
				(*out_normal)[1] = 0;
				(*out_normal)[2] = -step_z;
    		}
		}
	}
	return false; // no block hit within max_distance
}

void example_raycast(Game* game, Player* player, World* world) {
    vec3 origin = {
		player->camera.position[0],
    	player->camera.position[1],
        player->camera.position[2] 
	};
    vec3 direction = {
		player->camera.front[0],
        player->camera.front[1],
        player->camera.front[2]
	};

	// glm_vec3_normalize(origin);
	glm_vec3_normalize(direction);

    Block hit_block;
    ivec3 hit_coord;
    ivec3 hit_normal;
    float max_dist = 8.0f;

    if(raycast_voxels(world, origin, direction, max_dist, &hit_block, &hit_coord, &hit_normal)) {
		printf("hit!\n");

		Block selected_block;
		selected_block = player->selected_block;
		if(world_get_block(&game->world, hit_coord[0], hit_coord[1], hit_coord[2]) != selected_block) {
			world_set_block(&game->world, hit_coord[0], hit_coord[1], hit_coord[2], selected_block);
			world_rebuild(&game->world);

			// printf("Placing at (%d, %d, %d)\n", x, y, z);
			// printf("Current block: %d\n", world_get_block(world, x, y, z));
		}
	} else {
		printf("no hit!\n");
	}
	fflush(stdout);
}

void player_place_block(Game* game) {
	example_raycast(game, &game->player, &game->world);
	// player_set_block(game, game->player.selected_block);
}

void player_destroy_block(Game* game) {
	// player_set_block(game, BLOCK_AIR);
}

