#ifndef GAME_H
#define GAME_H

#include "player.h"
#include "debug.h"
#include "shader.h"
#include "render_context.h"
#include "world.h"
#include "input.h"
#include "entity.h"

typedef struct Game {
	GLFWwindow* window;
	float window_width;
	float window_height;
	float aspect;

	float last_time;
	float delta_time;
	float accumulator;
	float alpha;

	float last_x;
	float last_y;
	bool first_mouse;
					  
	Shader shader; // block shader
	RenderContext ctx;
	World world;
	Player player;

    bool debug_wireframe_mode;
    bool debug_backface_culling;
} Game;

int game_init(Game* game);
void game_run(Game* game);
void game_close(Game* game);

#endif // GAME_H
