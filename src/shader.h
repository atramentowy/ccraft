#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <stdbool.h>

typedef struct {
	unsigned int ID;
} Shader;

Shader shader_create(const char* vertex_path, const char* fragment_path);
void shader_use(Shader* shader);
void shader_set_bool(Shader* shader, const char* name, bool value);
void shader_set_int(Shader* shader, const char* name, int value);
void shader_set_float(Shader* shader, const char* name, float value);

#endif
