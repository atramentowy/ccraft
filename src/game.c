#include "game.h"

#include "filepath.h"
#include "texture.h"
#include "frustum.h"
#include "input.h"

int game_init(Game* game) {
	game->window_width = 800;
	game->window_height = 600;
	// input
	game->last_x = game->window_width / 2.0f;
	game->last_y = game->window_height / 2.0f;
	game->first_mouse = true;

	// set callbacks
	glfwSetWindowUserPointer(game->window, game);
	glfwSetErrorCallback(error_callback);
	glfwSetCursorPosCallback(game->window, mouse_callback);
    glfwSetScrollCallback(game->window, scroll_callback);
    glfwSetInputMode(game->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	
	// init glfw
	if(!glfwInit()) {
		fprintf(stderr, "GLFW: init failed\n");
		return -1;
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	// glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_SAMPLES, 4);

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

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);


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
	char* texture_path = make_path("res/atlas.png");
	path_make_absolute(texture_path, "res/atlas.png");
	Texture atlas = texture_create(texture_path, GL_TEXTURE_2D);
	texture_bind(&atlas, 0);
	free(texture_path);

	shader_set_int(&myShader, "blockTexture", 0);
	
	world_init(&game->world);
	world_rebuild(&game->world);
	player_init(&game->player);
}

void game_run(Game* game) {
	while(!glfwWindowShouldClose(game->window)) {
		float aspect = (float)game->window_width / (float)game->window_height;
		float now = (float)glfwGetTime();
		game->delta_time = now - game->last_frame;
		game->last_frame = now;

		process_input(game->window);

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		mat4 projection;
		glm_perspective(
			glm_rad(game->player.camera.zoom),
            aspect,
            game->player.near_plane,
			game->player.far_plane, projection
		);
		// shader_set_mat4(&myShader, "projection", projection);
		memcpy(game->ctx.projection, projection, sizeof(mat4));

		mat4 view;
		camera_get_view_matrix(&game->player.camera, view);
		// shader_set_mat4(&myShader, "view", view);
		memcpy(game->ctx.view, view, sizeof(mat4));
		
		Frustum frustum = create_frustum_from_camera(&game->player.camera, aspect, fov_y, game->player.near_plane, game->player.far_plane);
		game->ctx.frustum = frustum;

		// player_update(&player);
		shader_use(&game->shader);
		world_draw(&game->ctx, &game->world, &game->shader);

		glfwSwapBuffers(game->window);
		glfwPollEvents();
	}
}

void game_close(Game* game) {
	player_unload(&game->player);
	world_unload(&game->world);

	glfwDestroyWindow(game->window);
	glfwTerminate();
}
