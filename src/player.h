#ifndef PLAYER_H
#define PLAYER_H

#include "camera.h"
#include "frustum.h"

#define fov_y (60.0f * (M_PI / 180.0f))

typedef struct {
	Camera camera;
	float near_plane;
	float far_plane;
} Player;

void player_init(Player* player) {
	player->near_plane = 0.1f; // z near
	player->far_plane = 1000.0f; // z far
	
	vec3 position = {0.0f, 1.0f, 3.0f};
	vec3 up = {0.0f, 1.0f, 0.0f};
	camera_init(&player->camera, position, up, CAMERA_YAW, CAMERA_PITCH);
}

void player_update(Player* player);
void player_unload(Player* player);

#endif // PLAYER_H
