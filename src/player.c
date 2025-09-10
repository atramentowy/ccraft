#include "player.h"

void player_init(Player* player) {
	// player->near_plane = 0.1f; // z near
	// player->far_plane = 1000.0f; // z far
	
	vec3 position = {0.0f, 0.0f, 0.0f};
	vec3 up = {0.0f, 1.0f, 0.0f};
	camera_init(&player->camera, position, up, CAMERA_YAW, CAMERA_PITCH);
}

void player_place_block(Game* game) {

}
