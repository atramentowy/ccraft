#ifndef INPUT_H
#define INPUT_H

#include <glad.h>
#include <GLFW/glfw3.h>

void error_callback(int error, const char* description);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void scroll_callback(GLFWwindow* window, double _xoffset, double yoffset);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods); 

void process_input(GLFWwindow* window);

#endif // INPUT_H
