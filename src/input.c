#include "input.h"
#include "game.h"

Game game;

void error_callback(int error, const char* description) {
	fprintf(stderr, "GLFW Error (%d): %s\n", error, description);
}

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
