#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <cglm/cglm.h>

#include "filepath.h"
#include "render_context.h"
#include "shader.h"
#include "texture.h"
#include "camera.h"
#include "world.h"
#include "frustum.h"

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera;
vec3 position = {0.0f, 16.0f, 3.0f};
vec3 up       = {0.0f, 1.0f, 0.0f};

#define fov_y (60.0f * (M_PI / 180.0f))
//float fov_y = glm_rad(45.0f);

float z_near = 0.0f;
float z_far = 100.0f;

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float delta_time = 0.0f; // time between current frame and last frame
float last_frame = 0.0f;

void error_callback(int error, const char* description) {
	fprintf(stderr, "GLFW Error (%d): %s\n", error, description);
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera_process_keyboard(&camera, CAMERA_FORWARD,  delta_time);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera_process_keyboard(&camera, CAMERA_BACKWARD, delta_time);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera_process_keyboard(&camera, CAMERA_LEFT,     delta_time);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera_process_keyboard(&camera, CAMERA_RIGHT,    delta_time);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = (float)xpos;
        lastY = (float)ypos;
        firstMouse = false;
    }

    float xoffset = (float)xpos - lastX;
    float yoffset = lastY - (float)ypos; 
    lastX = (float)xpos;
    lastY = (float)ypos;

    camera_process_mouse(&camera, xoffset, yoffset, true);
}

void scroll_callback(GLFWwindow* window, double _xoffset, double yoffset) {
    camera_process_scroll(&camera, (float)yoffset);
}

int main() {
	glfwSetErrorCallback(error_callback);

	if(!glfwInit()) {
		fprintf(stderr, "GLFW: init failed\n");
		return -1;
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	// glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_SAMPLES, 4);

	GLFWwindow* window = glfwCreateWindow(800, 600, "ccraft", NULL, NULL);
	if(!window) {
		fprintf(stderr, "GLFW: failed to create window\n");
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		fprintf(stderr, "GLAD: failed to initialize\n");
		return -1;
	}

	glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	camera_init(&camera, position, up, CAMERA_YAW, CAMERA_PITCH);

	// todo: resource manager for shaders and textures
	//
	char* vert_path = make_path("res/shaders/shader.vert");
	char* frag_path = make_path("res/shaders/shader.frag");

	path_make_absolute(vert_path, "res/shaders/shader.vert");
	path_make_absolute(frag_path, "res/shaders/shader.frag");

	Shader myShader = shader_create(
		vert_path,
   	    frag_path
	);
	shader_use(&myShader);

	free(vert_path);
	free(frag_path);

	// texture atlas
	char* texture_path = make_path("res/atlas.png");
	path_make_absolute(texture_path, "res/atlas.png");
	Texture atlas = texture_create(texture_path, GL_TEXTURE_2D);
	texture_bind(&atlas, 0);

	free(texture_path);

	shader_set_int(&myShader, "blockTexture", 0);

	World world;
	world_init(&world);
	world_rebuild(&world);
	
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	
	RenderContext ctx;
	while(!glfwWindowShouldClose(window)) {
		float now = (float)glfwGetTime();
		delta_time = now - last_frame;
		last_frame = now;

		processInput(window);

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		mat4 projection;
		glm_perspective(
			glm_rad(camera.zoom),
            (float)SCR_WIDTH / SCR_HEIGHT,
            0.1f, 100.0f, projection
		);
		// shader_set_mat4(&myShader, "projection", projection);
		memcpy(ctx.projection, projection, sizeof(mat4));

		mat4 view;
		camera_get_view_matrix(&camera, view);
		// shader_set_mat4(&myShader, "view", view);
		memcpy(ctx.view, view, sizeof(mat4));

		Frustum frustum = create_frustum_from_camera(&camera, (float)SCR_WIDTH / (float)SCR_HEIGHT, fov_y, z_near, z_far);
		ctx.frustum = frustum;	

		world_draw(&ctx, &world, &myShader);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	world_unload(&world);
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
