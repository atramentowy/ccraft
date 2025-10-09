#include "game.h"

#include "filepath.h"
#include "texture.h"

int game_init(Game* game) {
	game->window_width = 800;
	game->window_height = 600;
	game->last_time = (float)glfwGetTime();
	game->last_x = game->window_width / 2.0f;
	game->last_y = game->window_height / 2.0f;
	game->first_mouse = true;
	
	// physics
	game->accumulator = 0.0f;

    game->debug_wireframe_mode = false;
    game->debug_backface_culling = true;

	// init glfw
	if(!glfwInit()) {
		fprintf(stderr, "GLFW: init failed\n");
		return -1;
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	// glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    // glfwWindowHint(GLFW_SAMPLES, 16); // 4x MSAA // does not work for voxels


	game->window = glfwCreateWindow(game->window_width, game->window_height, "ccraft", NULL, NULL);
	if(!game->window) {
		fprintf(stderr, "GLFW: failed to create window\n");
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(game->window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		fprintf(stderr, "GLAD: failed to initialize\n");
		return -1;
	}

	// set callbacks
	glfwSetWindowUserPointer(game->window, game);
	glfwSetErrorCallback(error_callback);
	glfwSetCursorPosCallback(game->window, mouse_callback);
	glfwSetMouseButtonCallback(game->window, mouse_button_callback);	
    glfwSetScrollCallback(game->window, scroll_callback);
	glfwSetKeyCallback(game->window, key_callback);
	// change mouse cursor visibility
	glfwSetInputMode(game->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
    // glEnable(GL_BLEND);
	
	// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    //
	// shaders
    char* vert_path = make_path("res/shaders/shader.vert");
	char* frag_path = make_path("res/shaders/shader.frag");
	path_make_absolute(vert_path, "res/shaders/shader.vert");
	path_make_absolute(frag_path, "res/shaders/shader.frag");

	Shader myShader = shader_create(
		vert_path,
   	    frag_path
	);
	shader_use(&myShader);
	game->shader = myShader;

	free(vert_path);
	free(frag_path);

	// textures atlas
	char* texture_path = make_path("res/textures.png");
	path_make_absolute(texture_path, "res/textures.png");
	Texture atlas = texture_create(texture_path, GL_TEXTURE_2D);
	texture_bind(&atlas, 0);
	free(texture_path);
	shader_set_int(&myShader, "blockTexture", 0);
	
	world_init(&game->world);
	world_rebuild(&game->world);
	player_init(&game->player);

    return 0;
}

void game_run(Game* game) {
	while(!glfwWindowShouldClose(game->window)) {
		game->aspect = (float)game->window_width / (float)game->window_height;
		
	    float current_time = (float)glfwGetTime();
    	game->delta_time = current_time - game->last_time;
    	game->last_time = current_time;
		game->accumulator += game->delta_time;
		game->alpha = game->accumulator / PHYSICS_TIMESTEP;
		if (game->alpha > 1.0f) game->alpha = 1.0f;

		// input
		glfwPollEvents();
		process_input(game->window);

		// process physics
		while (game->accumulator >= PHYSICS_TIMESTEP) {
			// entity_update(&game->world, &game->entity, PHYSICS_TIMESTEP);
			entity_update(&game->world, &game->player.entity, PHYSICS_TIMESTEP);
			game->accumulator -= PHYSICS_TIMESTEP;
		}

		// draw
		glClearColor(0.5098f, 0.7843f, 0.8980f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shader_use(&game->shader);
		player_update(&game->player, game);
		world_draw(&game->ctx, &game->world, &game->shader);

		glfwSwapBuffers(game->window);
	}
}

void game_close(Game* game) {
	// player_unload(&game->player);
	world_unload(&game->world);

	glfwDestroyWindow(game->window);
	glfwTerminate();
}
