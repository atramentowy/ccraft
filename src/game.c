#include "game.h"

#include "filepath.h"
#include "texture.h"
// #include "frustum.h"

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
	glfwSetMouseButtonCallback(game->window, mouse_button_callback);	
    glfwSetScrollCallback(game->window, scroll_callback);
	glfwSetKeyCallback(game->window, key_callback);
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

		glfwPollEvents();
		process_input(game->window);

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
