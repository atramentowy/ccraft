#ifndef PLAYER_H
#define PLAYER_H

#include "camera.h"

typedef struct Game Game;

typedef struct {
	Camera camera;
	// Frustum frustum;
} Player;

void player_init(Player* player);

void player_place_block(Game* game);
// void player_place_block(Player* player);

// void player_update(Player* player);
// void player_unload(Player* player);

#endif // PLAYER_H
