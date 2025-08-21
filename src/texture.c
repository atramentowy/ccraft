#include "texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static void texture_load(Texture* texture) {
    glGenTextures(1, &texture->ID);
    glBindTexture(texture->texture_type, texture->ID);

    // Texture parameters
	glTexParameteri(texture->texture_type, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(texture->texture_type, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(texture->texture_type, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(texture->texture_type, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    int width, height, nr_channels;
    stbi_set_flip_vertically_on_load(1);
    unsigned char* data = stbi_load(texture->file_path, &width, &height, &nr_channels, 0);
    if (data) {
        GLenum format;
        if (nr_channels == 1)
            format = GL_RED;
        else if (nr_channels == 3)
            format = GL_RGB;
        else if (nr_channels == 4)
            format = GL_RGBA;
        else {
            stbi_image_free(data);
            fprintf(stderr, "TEXTURE: Unsupported texture format: %d channels\n", nr_channels);
            return;
        }

        glTexImage2D(texture->texture_type, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(texture->texture_type);
    } else {
        fprintf(stderr, "TEXTURE: Failed to load texture at path: %s\n", texture->file_path);
    }

    stbi_image_free(data);
}

Texture texture_create(const char* file_path, GLenum texture_type) {
    Texture texture;
    texture.texture_type = texture_type;
    texture.file_path = file_path;
    texture.ID = 0;

    texture_load(&texture);
    return texture;
}
void texture_bind(const Texture* texture, GLuint unit) {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(texture->texture_type, texture->ID);
}

void texture_unbind(const Texture* texture) {
    glBindTexture(texture->texture_type, 0);
}

void texture_destroy(Texture* texture) {
    glDeleteTextures(1, &texture->ID);
    texture->ID = 0;
}
