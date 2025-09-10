#ifndef INPUT_H
#define INPUT_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define MIN_VALID_KEY GLFW_KEY_SPACE
#define MAX_KEYS (GLFW_KEY_LAST + 1)
#define MAX_MOUSE_BUTTONS 8

typedef struct {
	GLFWwindow* window;
	int prev_key_state[MAX_KEYS];
	int prev_mouse_state[MAX_MOUSE_BUTTONS];
} Input;

void input_init(Input* input, GLFWwindow* window);
void input_update(Input* input);
void input_unload(Input* input);

// int is_key_down(int key);
// int is_key_just_pressed(int key);
// int is_key_released(int key);
//
int mouse_just_pressed(Input* input, int button);

void error_callback(int error, const char* description);
// void process_input(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double _xoffset, double yoffset);

#endif // INPUT_H
