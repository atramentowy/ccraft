#ifndef ENTITY_H
#define ENTITY_H

#include "world.h"

#include <glad.h>
#include <cglm/cglm.h>
#include <stdbool.h>

#define GRAVITY -9.81f
#define PHYSICS_TIMESTEP (1.0f / 60.0f)

typedef struct {
	vec3 prev_position; // position in last frame
    vec3 position;
    vec3 velocity;
    vec3 rotation;
    float mass;
    float width;
    float height;
    float depth;
    bool is_on_ground;
	bool was_on_ground;
} Entity;

typedef struct {
    vec3 min;
    vec3 max;
} AABB;

void entity_apply_force(Entity* entity, vec3 force);
void entity_init(Entity* entity, vec3 position, float mass, float width, float height, float depth);
void entity_update(World* world, Entity* entity, float timestep);

#endif // ENTITY_H

