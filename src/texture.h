#ifndef TEXTURE_H
#define TEXTURE_H

#include <glad.h>

typedef struct {
	GLuint ID;
	GLenum texture_type;
	const char* file_path;
} Texture;

Texture texture_create(const char* filePath, GLenum type);
void texture_bind(const Texture* texture, GLuint unit);
void texture_unbind(const Texture* texture);
void texture_destroy(Texture* texture);

#endif // TEXTURE_H
