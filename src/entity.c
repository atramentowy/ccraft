#include "entity.h"

bool collides_with_voxels(void* world, const Entity* entity) {
    float x = entity->position[0];
    float y = entity->position[1];
    float z = entity->position[2];

    int min_x = (int)floorf(x);
    int max_x = (int)ceilf(x + entity->width);
    int min_y = (int)floorf(y);
    int max_y = (int)ceilf(y + entity->height);
    int min_z = (int)floorf(z);
    int max_z = (int)ceilf(z + entity->depth);

    for (int i = min_x; i < max_x; ++i) {
        for (int j = min_y; j < max_y; ++j) {
            for (int k = min_z; k < max_z; ++k) {
                if (world_get_block(world, i, j, k) != BLOCK_AIR) {
                    return true;
                }
            }
        }
    }
    return false;
}

void entity_init(Entity* entity, vec3 position, float mass, float width, float height, float depth) {
    glm_vec3_copy(position, entity->position);
    glm_vec3_zero(entity->velocity);
    glm_vec3_zero(entity->rotation);

    entity->mass = mass;
    entity->width = width;
    entity->height = height;
    entity->depth = depth;
    entity->is_on_ground = false;
}

void entity_update(World* world, Entity* entity, float timestep) {
    // apply gravity
    if (!entity->is_on_ground) {
        entity->velocity[1] += GRAVITY * timestep;
    }

    // predict next position
    vec3 next_pos;
    vec3 scaled_vel;
    glm_vec3_scale(entity->velocity, timestep, scaled_vel);
    glm_vec3_add(entity->position, scaled_vel, next_pos);

    // y-axis collision
    {
        Entity temp = *entity;
        temp.position[1] = next_pos[1];

        if (collides_with_voxels(world, &temp)) {
            if (entity->velocity[1] < 0.0f) {
                entity->is_on_ground = true;
                entity->velocity[1] = 0.0f;
                entity->position[1] = floorf(entity->position[1] + entity->height / 2.0f);
            } else {
                entity->velocity[1] = 0.0f;
            }
        } else {
            entity->is_on_ground = false;
            entity->position[1] = next_pos[1];
        }
    }

    // x-axis collision
    {
        Entity temp = *entity;
        temp.position[0] = next_pos[0];

        if (collides_with_voxels(world, &temp)) {
            if (entity->velocity[0] > 0.0f) {
                entity->position[0] = floorf(entity->position[0] + entity->width / 2.0f);
            } else if (entity->velocity[0] < 0.0f) {
                entity->position[0] = ceilf(entity->position[0] - entity->width / 2.0f);
            }
            entity->velocity[0] = 0.0f;
        } else {
            entity->position[0] = next_pos[0];
        }
    }

    // z-axis collision
    {
        Entity temp = *entity;
        temp.position[2] = next_pos[2];

        if (collides_with_voxels(world, &temp)) {
            if (entity->velocity[2] > 0.0f) {
                entity->position[2] = floorf(entity->position[2] + entity->depth / 2.0f);
            } else if (entity->velocity[2] < 0.0f) {
                entity->position[2] = ceilf(entity->position[2] - entity->depth / 2.0f);
            }
            entity->velocity[2] = 0.0f;
        } else {
            entity->position[2] = next_pos[2];
        }
    }
}

