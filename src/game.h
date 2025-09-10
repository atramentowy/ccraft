#ifndef GAME_H
#define GAME_H

#include "player.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdbool.h>

#include "shader.h"
#include "render_context.h"
#include "world.h"
#include "input.h"

typedef struct Game {
	GLFWwindow* window;
	float window_width;
	float window_height;

	float delta_time;
	float last_frame;

	float last_x;
	float last_y;
	bool first_mouse;
					  
	Shader shader; // block shader

	RenderContext ctx;
	World world;
	Player player;
} Game;

int game_init(Game* game);
void game_run(Game* game);
void game_close(Game* game);

#endif // GAME_H
