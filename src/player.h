#ifndef PLAYER_H
#define PLAYER_H

#include "camera.h"
#include "frustum.h"
#include "chunk.h"

typedef struct Game Game;
typedef struct Chunk Chunk;
typedef enum Block Block;

typedef struct {
	Camera camera;
	Frustum frustum;
	Block selected_block;
} Player;

void player_init(Player* player);
void player_update(Player* player, Game* game);
// void player_unload(Player* player);

void player_place_block(Game* game);
void player_destroy_block(Game* game);

#endif // PLAYER_H
