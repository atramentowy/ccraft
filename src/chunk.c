#include "chunk.h"
#include <glad/glad.h>
#include <cglm/cglm.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

float vertices[] = {
    -0.5f, -0.5f, 0.0f,
     0.5f, -0.5f, 0.0f,
     0.0f,  0.5f, 0.0f
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

    chunk->vao = chunk->vbo = chunk->ebo = 0;
}

void chunk_unload(Chunk* chunk) {
    if (chunk->vao) glDeleteVertexArrays(1, &chunk->vao);
    if (chunk->vbo) glDeleteBuffers(1, &chunk->vbo);
    if (chunk->ebo) glDeleteBuffers(1, &chunk->ebo);

    chunk->vao = chunk->vbo = chunk->ebo = 0;
}

void chunk_rebuild(Chunk* chunk) {
	glGenVertexArrays(1, &chunk->vao);
    glGenBuffers(1, &chunk->vbo);

	glBindVertexArray(chunk->vao);
	
	glBindBuffer(GL_ARRAY_BUFFER, chunk->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0); 
	
	glBindVertexArray(0); 	
}

void chunk_draw(const Chunk* chunk, Shader* shader) {
	// shader_use(shader);

	glBindVertexArray(chunk->vao);
	// glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunk->ebo);

	glDrawArrays(GL_TRIANGLES, 0, 3);

	glBindVertexArray(0);
}
