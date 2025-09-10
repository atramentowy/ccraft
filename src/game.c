#include "game.h"

#include "filepath.h"
#include "texture.h"
#include "frustum.h"

/*
int raycast(World* world, float ox, float oy, float oz, float dx, float dy, float dz,
            int* out_x, int* out_y, int* out_z, float max_distance) {
    const float step = 0.05f;

    for (float t = 0.0f; t < max_distance; t += step) {
        int x = (int)floorf(ox + dx * t);
        int y = (int)floorf(oy + dy * t);
        int z = (int)floorf(oz + dz * t);

        if (x < 0 || x >= WORLD_SIZE_X ||
            y < 0 || y >= WORLD_SIZE_Y ||
            z < 0 || z >= WORLD_SIZE_Z)
            continue;

        if (world_get_block(world, x, y, z) != BLOCK_AIR) {
            int px = x - (dx > 0 ? 1 : (dx < 0 ? -1 : 0));
            int py = y - (dy > 0 ? 1 : (dy < 0 ? -1 : 0));
            int pz = z - (dz > 0 ? 1 : (dz < 0 ? -1 : 0));

            if (px >= 0 && px < WORLD_SIZE_X &&
                py >= 0 && py < WORLD_SIZE_Y &&
                pz >= 0 && pz < WORLD_SIZE_Z) {
                *out_x = px;
                *out_y = py;
                *out_z = pz;
                return 1;
            } else {
                return 0; // Hit block, but can't place
            }
        }
    }
    return 0;
}
*/

void game_process_input(Game* game) {
	if (glfwGetKey(game->window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(game->window, true);

    if (glfwGetKey(game->window, GLFW_KEY_W) == GLFW_PRESS)
        camera_process_keyboard(&game->player.camera, CAMERA_FORWARD,  game->delta_time);
    if (glfwGetKey(game->window, GLFW_KEY_S) == GLFW_PRESS)
        camera_process_keyboard(&game->player.camera, CAMERA_BACKWARD, game->delta_time);
    if (glfwGetKey(game->window, GLFW_KEY_A) == GLFW_PRESS)
        camera_process_keyboard(&game->player.camera, CAMERA_LEFT,     game->delta_time);
    if (glfwGetKey(game->window, GLFW_KEY_D) == GLFW_PRESS)
        camera_process_keyboard(&game->player.camera, CAMERA_RIGHT,    game->delta_time);
	
	if (mouse_just_pressed(&game->input, GLFW_MOUSE_BUTTON_RIGHT)) {		
		vec3 origin;
		glm_vec3_copy(game->player.camera.position, origin);

		int x = (int)roundf(origin[0]);
		int y = (int)roundf(origin[1]);
		int z = (int)roundf(origin[2]);
		
		// also check if block is not already there
		if (world_get_block(&game->world, x, y, z) != BLOCK_GRASS) {
			world_set_block(&game->world, x, y, z, BLOCK_GRASS);
			world_rebuild(&game->world);
		}

		// printf("Placing at (%d, %d, %d)\n", x, y, z);
		// printf("Current block: %d\n", world_get_block(world, x, y, z));
    } 
	else if (mouse_just_pressed(&game->input, GLFW_MOUSE_BUTTON_LEFT)) {
		vec3 origin;
		glm_vec3_copy(game->player.camera.position, origin);

		int x = (int)roundf(origin[0]);
		int y = (int)roundf(origin[1]);
		int z = (int)roundf(origin[2]);

		if (world_get_block(&game->world, x, y, z) != BLOCK_AIR) {
			world_set_block(&game->world, x, y, z, BLOCK_AIR);
			world_rebuild(&game->world);
		}
		// printf("Placing at (%d, %d, %d)\n", x, y, z);
		// printf("Current block: %d\n", world_get_block(world, x, y, z));
	}
}

int game_init(Game* game) {
	game->window_width = 800;
	game->window_height = 600;
	// input
	game->last_x = game->window_width / 2.0f;
	game->last_y = game->window_height / 2.0f;
	game->first_mouse = true;

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

	// set callbacks
	glfwSetWindowUserPointer(game->window, game);
	glfwSetErrorCallback(error_callback);
	glfwSetCursorPosCallback(game->window, mouse_callback);
	// glfwSetMouseButtonCallback(window, mouse_button_callback);	
    glfwSetScrollCallback(game->window, scroll_callback);
	// change mouse cursor visibility
	glfwSetInputMode(game->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

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
	
	input_init(&game->input, game->window);
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

		input_update(&game->input);
		glfwPollEvents();
		// process_input(game->window);
		// input_update(&game->input);
		game_process_input(game);

		glClearColor(0.5098f, 0.7843f, 0.8980f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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

		// player_update(&player);
		shader_use(&game->shader);
		world_draw(&game->ctx, &game->world, &game->shader);

		glfwSwapBuffers(game->window);
	}
}

void game_close(Game* game) {
	// player_unload(&game->player);
	input_unload(&game->input);
	world_unload(&game->world);

	glfwDestroyWindow(game->window);
	glfwTerminate();
}
