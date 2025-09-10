#include "input.h"
#include <stdio.h>
#include "game.h"

void input_init(Input* input, GLFWwindow* window) {
	input->window = window;
	for (int i = MIN_VALID_KEY; i < MAX_KEYS; i++) {
		input->prev_key_state[i] = GLFW_RELEASE;
	}
}

void input_update(Input* input) {
	for(int key = MIN_VALID_KEY; key < MAX_KEYS; key++) {
		input->prev_key_state[key] = glfwGetKey(input->window, key);
	}
	for(int button = 0; button < MAX_MOUSE_BUTTONS; button++) {
		input->prev_mouse_state[button] = glfwGetMouseButton(input->window, button);
	}
}

void input_unload(Input* input) {

}

int mouse_just_pressed(Input* input, int button) {
	int current = glfwGetMouseButton(input->window, button);
	int previous = input->prev_mouse_state[button];

	input->prev_mouse_state[button] = current;

	return current == GLFW_PRESS && previous == GLFW_RELEASE;
}

void error_callback(int error, const char* description) {
	fprintf(stderr, "GLFW Error (%d): %s\n", error, description);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	Game* game = (Game*)glfwGetWindowUserPointer(window);

    if (game->first_mouse) {
        game->last_x = (float)xpos;
        game->last_y = (float)ypos;
        game->first_mouse = false;
    }

    float xoffset = (float)xpos - game->last_x;
    float yoffset = game->last_y - (float)ypos; 
    game->last_x = (float)xpos;
    game->last_y = (float)ypos;

    camera_process_mouse(&game->player.camera, xoffset, yoffset, true);
}

void scroll_callback(GLFWwindow* window, double _xoffset, double yoffset) {
	Game* game = (Game*)glfwGetWindowUserPointer(window);

    camera_process_scroll(&game->player.camera, (float)yoffset);
}
/*
void process_input(GLFWwindow* window) {
	Game* game = (Game*)glfwGetWindowUserPointer(window);

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera_process_keyboard(&game->player.camera, CAMERA_FORWARD,  game->delta_time);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera_process_keyboard(&game->player.camera, CAMERA_BACKWARD, game->delta_time);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera_process_keyboard(&game->player.camera, CAMERA_LEFT,     game->delta_time);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera_process_keyboard(&game->player.camera, CAMERA_RIGHT,    game->delta_time);

	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
    	player_place_block(&game->player);
	}
}

void is_key_pressed(int key) {
	return glfwGetKey(GLFWwindow* window, key) == GLFW_PRESS;
}

void is_mouse_button_pressed(int button) {
	return glfwGetKey(GLFWwindow* window, button) == GLFW_PRESS;
}

void get_mouse_position(double x, double y) {

}
*/


