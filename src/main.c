#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <cglm/cglm.h>

#include "shader.h"
#include "camera.h"

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera;
vec3 position = {0.0f, 0.0f, 3.0f};
vec3 up       = {0.0f, 1.0f, 0.0f};

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

void error_callback(int error, const char* description) {
	fprintf(stderr, "GLFW Error (%d): %s\n", error, description);
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera_process_keyboard(&camera, CAMERA_FORWARD,  deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera_process_keyboard(&camera, CAMERA_BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera_process_keyboard(&camera, CAMERA_LEFT,     deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera_process_keyboard(&camera, CAMERA_RIGHT,    deltaTime);
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

	// shader
	Shader myShader = shader_create("../res/shaders/camera.vert", "../res/shaders/camera.frag");
	shader_use(&myShader);

	float vertices[] = {
	    0.5f,  0.5f, 0.0f,  // top right
	    0.5f, -0.5f, 0.0f,  // bottom right
           -0.5f, -0.5f, 0.0f,  // bottom left
           -0.5f,  0.5f, 0.0f   // top left 
    	};

    	unsigned int indices[] = {  // note that we start from 0!
    	    0, 1, 3,  // first Triangle
    	    1, 2, 3   // second Triangle
    	};
    	unsigned int VBO, VAO, EBO;
    	glGenVertexArrays(1, &VAO);
    	glGenBuffers(1, &VBO);
    	glGenBuffers(1, &EBO);
    	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    	glBindVertexArray(VAO);

    	glBindBuffer(GL_ARRAY_BUFFER, VBO);
    	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    	glEnableVertexAttribArray(0);

    	// note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    	glBindBuffer(GL_ARRAY_BUFFER, 0); 

    	// remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
    	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    	// You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    	// VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    	glBindVertexArray(0);
	glBindVertexArray(VAO);

	glEnable(GL_DEPTH_TEST);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	while(!glfwWindowShouldClose(window)) {
		float now = (float)glfwGetTime();
    		deltaTime = now - lastFrame;
    		lastFrame = now;

		processInput(window);

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shader_use(&myShader);

		mat4 projection;       
		glm_perspective(
		    glm_rad(camera.Zoom),
                    (float)SCR_WIDTH/SCR_HEIGHT,
                    0.1f, 100.0f, projection
		);
		shader_set_mat4(&myShader, "projection", projection);

        	mat4 view;
        	camera_get_view_matrix(&camera, view);
        	shader_set_mat4(&myShader, "view", view);

		mat4 model;
    		glm_mat4_identity(model);
    		shader_set_mat4(&myShader, "model", model);

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		GLenum err = glGetError();
		if (err != GL_NO_ERROR)
    			fprintf(stderr, "GL error: 0x%X\n", err);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
