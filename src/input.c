#include "input.h"
#include <stdio.h>
#include "game.h"
#include "entity.h"
#include "player.h"

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

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	Game* game = (Game*)glfwGetWindowUserPointer(window);

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
        player_destroy_block(game);
	else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
		player_place_block(game);
}

void scroll_callback(GLFWwindow* window, double _xoffset, double yoffset) {
	Game* game = (Game*)glfwGetWindowUserPointer(window);

    camera_process_scroll(&game->player.camera, (float)yoffset);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	Game* game = (Game*)glfwGetWindowUserPointer(window);

    if (action == GLFW_PRESS) {
        switch (key) {
        	case GLFW_KEY_1:
                game->player.selected_block = BLOCK_DIRT;
                break;
            case GLFW_KEY_2:
                break;
            case GLFW_KEY_3:
                break;
            case GLFW_KEY_4:
                break;
            case GLFW_KEY_5:
                break;
            case GLFW_KEY_6:
                break;
            case GLFW_KEY_7:
                break;
            case GLFW_KEY_8:
                break;
            case GLFW_KEY_9:
                break;
            case GLFW_KEY_0:
                break;
        }
    }
}

void process_input(GLFWwindow* window) {
	Game* game = (Game*)glfwGetWindowUserPointer(window);

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(game->window, true);
	
	// player movement
    vec3 move_dir;
    glm_vec3_zero(move_dir);

    vec3 forward, right;
    glm_vec3_copy(game->player.camera.front, forward);
    glm_vec3_copy(game->player.camera.right, right);
    forward[1] = 0.0f;
    right[1] = 0.0f;
    glm_normalize(forward);
    glm_normalize(right);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        glm_vec3_add(move_dir, forward, move_dir);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        glm_vec3_sub(move_dir, forward, move_dir);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        glm_vec3_add(move_dir, right, move_dir);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        glm_vec3_sub(move_dir, right, move_dir);

	float move_force = (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) ? 8.0f : 5.0f;
    if (!glm_vec3_eqv(move_dir, GLM_VEC3_ZERO)) {
        glm_normalize(move_dir); // prevent diagonal speed boost
        glm_vec3_scale(move_dir, move_force, move_dir); // scale by movement force
        entity_apply_force(&game->player.entity, move_dir); // apply as force
    }

    // Clamp velocity to max speed
    float max_speed = 10.0f;
    float speed = glm_vec3_norm(game->player.entity.velocity);
    if (speed > max_speed) {
        glm_vec3_normalize(game->player.entity.velocity);
        glm_vec3_scale(game->player.entity.velocity, max_speed, game->player.entity.velocity);
    }
}

