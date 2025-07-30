#include "shader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char* read_file(const char* path) {
	FILE* file = fopen(path, "rb");
	if(!file) {
		fprintf(stderr, "ERROR::FAILED TO OPEN FILE: %s\n", path);
		return NULL;
	}

	fseek(file, 0, SEEK_END);
	long length = ftell(file);
	rewind(file);

	char* buffer = (char*)malloc(length + 1);
	if(!buffer) {
		fclose(file);
		return NULL;
	}

	fread(buffer, 1, length, file);
	buffer[length] = '\0';
	fclose(file);
	return buffer;
}

static void check_compile_errors(unsigned int shader, const char* type, const char* path) {
	int success;
	char info_log[1024];
	if(strcmp(type, "PROGRAM") != 0) {
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if(!success) {
			glGetShaderInfoLog(shader, 1024, NULL, info_log);
			fprintf(stderr, "%s: ERROR::SHADER_COMPILATION_ERROR of type %s\n%s\n", path, type, info_log);
		} else {
			glGetProgramiv(shader, GL_LINK_STATUS, &success);
			if(!success) {
				glGetProgramInfoLog(shader, 1024, NULL, info_log);
				fprintf(stderr, "%s: ERROR::PROGRAM_LINKING_ERROR\n%s\n", path, info_log);
			}
		}
	}
}

Shader shader_create(const char* vertex_path, const char* fragment_path) {
	Shader shader;
	char* vertex_code = read_file(vertex_path);
	char* fragment_code = read_file(fragment_path);

	if(!vertex_code || !fragment_code) {
		shader.ID = 0;
		return shader;
	}

	unsigned int vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, (const char**)&vertex_code, NULL);
	glCompileShader(vertex);
	check_compile_errors(vertex, "VERTEX", vertex_path);

	unsigned int fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, (const char**)&fragment_code, NULL);
	glCompileShader(fragment);
	check_compile_errors(fragment, "FRAGMENT", fragment_path);

	shader.ID = glCreateProgram();
	glAttachShader(shader.ID, vertex);
	glAttachShader(shader.ID, fragment);
	glLinkProgram(shader.ID);
	check_compile_errors(shader.ID, "PROGRAM", "shader_program");

	glDeleteShader(vertex);
	glDeleteShader(fragment);

	free(vertex_code);
	free(fragment_code);

	return shader;
}

void shader_use(Shader* shader) {
	glUseProgram(shader->ID);
}

void shader_set_bool(Shader* shader, const char* name, bool value) {
	GLint loc = glGetUniformLocation(shader->ID, name);
    if (loc == -1) {
        fprintf(stderr, "WARNING: uniform '%s' not found in shader\n", name);
        return;
    }
	glUniform1i(glGetUniformLocation(shader->ID, name), value);
}

void shader_set_int(Shader* shader, const char* name, int value) {
	GLint loc = glGetUniformLocation(shader->ID, name);
    if (loc == -1) {
        fprintf(stderr, "WARNING: uniform '%s' not found in shader\n", name);
        return;
    }
	glUniform1i(glGetUniformLocation(shader->ID, name), value);
}

void shader_set_float(Shader* shader, const char* name, float value) {
	GLint loc = glGetUniformLocation(shader->ID, name);
    if (loc == -1) {
        fprintf(stderr, "WARNING: uniform '%s' not found in shader\n", name);
        return;
    }
	glUniform1f(glGetUniformLocation(shader->ID, name), value);
}

void shader_set_mat4(Shader* shader, const char* name, const mat4 matrix) {
    GLint loc = glGetUniformLocation(shader->ID, name);
	if (loc == -1) {
        fprintf(stderr, "WARNING: uniform '%s' not found in shader\n", name);
        return;
    }
    glUniformMatrix4fv(loc, 1, GL_FALSE, (const float*)matrix);
}
