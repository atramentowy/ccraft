#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>

#include "shader.h"

void error_callback(int error, const char* description) {
	fprintf(stderr, "GLFW Error (%d): %s\n", error, description);
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

	// shader
	Shader myShader = shader_create("../res/shaders/shader.vert", "../res/shaders/shader.frag");
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

	while(!glfwWindowShouldClose(window)) {
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
