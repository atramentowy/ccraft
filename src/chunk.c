#include "chunk.h"
#include <glad/glad.h>
#include <cglm/cglm.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

float cube_vertices[] = {
    // Positions          
    -0.5f, -0.5f, -0.5f,  
     0.5f, -0.5f, -0.5f,  
     0.5f,  0.5f, -0.5f,  
    -0.5f,  0.5f, -0.5f,  

    -0.5f, -0.5f,  0.5f,  
     0.5f, -0.5f,  0.5f,  
     0.5f,  0.5f,  0.5f,  
    -0.5f,  0.5f,  0.5f   
};

unsigned int cube_indices[] = {
    // Back face
    0, 1, 2, 2, 3, 0,
    // Front face
    4, 5, 6, 6, 7, 4,
    // Left face
    4, 0, 3, 3, 7, 4,
    // Right face
    1, 5, 6, 6, 2, 1,
    // Bottom face
    4, 5, 1, 1, 0, 4,
    // Top face
    3, 2, 6, 6, 7, 3
};

void chunk_init(Chunk* chunk) {
    for (int x = 0; x < CHUNK_SIZE; x++)
    	for (int y = 0; y < CHUNK_SIZE; y++)
    		for (int z = 0; z < CHUNK_SIZE; z++)
        		chunk->blocks[x][y][z] = Air;

    // simple ground layer
    for (int x = 0; x < CHUNK_SIZE; x++)
    	for (int z = 0; z < CHUNK_SIZE; z++)
        	chunk->blocks[x][0][z] = Grass;


	// initialize
	chunk->vao = chunk->vbo = chunk->ebo = 0;
	//
	glGenVertexArrays(1, &chunk->vao);
    glGenBuffers(1, &chunk->vbo);
	glGenBuffers(1, &chunk->ebo);

	glBindVertexArray(chunk->vao);
	
	glBindBuffer(GL_ARRAY_BUFFER, chunk->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunk->ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_indices), cube_indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0); 
	glBindVertexArray(0);
}

void chunk_unload(Chunk* chunk) {
    if (chunk->vao) glDeleteVertexArrays(1, &chunk->vao);
    if (chunk->vbo) glDeleteBuffers(1, &chunk->vbo);
    if (chunk->ebo) glDeleteBuffers(1, &chunk->ebo);

    chunk->vao = chunk->vbo = chunk->ebo = 0;
}

void chunk_rebuild(Chunk* chunk) {
	
}

void chunk_draw(const Chunk* chunk, Shader* shader) {
	glBindVertexArray(chunk->vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunk->ebo);
	
	// shader_use(shader);
	glDrawElements(GL_TRIANGLES, sizeof(cube_indices)/sizeof(GLuint), GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);
}
