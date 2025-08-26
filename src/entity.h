#ifndef ENTITY_H
#define ENTITY_H

#include <cglm/cglm.h>
#include <stdbool.h>

typedef struct {
	vec3 position;
	vec3 velocity;

	bool is_grounded;
	float width; // collision box
	float height;
} Entity;

void entity_init(Entity* entity, vec3 position, float width, float height) {
	glm_vec3_copy(position, entity->position);
	glm_vec3_zero(entity->velocity);
	entity->is_grounded = false;
	entity->width = width;
	entity->height = height;
}

// void entity_draw();

#endif // ENTITY_H
