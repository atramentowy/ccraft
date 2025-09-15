#include "entity.h"

#define GROUND_TOLERANCE 0.05f

bool collides_with_voxels(void* world, const Entity* entity) {
    float x = entity->position[0];
    float y = entity->position[1];
    float z = entity->position[2];

    float half_width  = entity->width  * 0.5f;
    float half_height = entity->height * 0.5f;
    float half_depth  = entity->depth  * 0.5f;

    // Entity's AABB in world space
    float min_x = x - half_width;
    float max_x = x + half_width;

    float min_y = y - half_height;
    float max_y = y + half_height;

    float min_z = z - half_depth;
    float max_z = z + half_depth;

    // Adjust to voxel coordinates where blocks are centered at (i,j,k)
    // and each block spans [-0.5, +0.5) around its center
    int voxel_min_x = (int)floorf(min_x + 0.5f);
    int voxel_max_x = (int)ceilf (max_x - 0.5f);

    int voxel_min_y = (int)floorf(min_y + 0.5f);
    int voxel_max_y = (int)ceilf (max_y - 0.5f);

    int voxel_min_z = (int)floorf(min_z + 0.5f);
    int voxel_max_z = (int)ceilf (max_z - 0.5f);

    // Loop over all blocks the AABB touches
    for (int i = voxel_min_x; i <= voxel_max_x; ++i) {
        for (int j = voxel_min_y; j <= voxel_max_y; ++j) {
            for (int k = voxel_min_z; k <= voxel_max_z; ++k) {
                if (world_get_block(world, i, j, k) != BLOCK_AIR) {
                    return true;
                }
            }
        }
    }

    return false;
}

void entity_apply_force(Entity* entity, vec3 force) {
	vec3 acceleration;
    glm_vec3_scale(force, 1.0f / entity->mass, acceleration);

    glm_vec3_add(entity->velocity, acceleration, entity->velocity);	
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
	entity->was_on_ground = false;
}

void entity_update(World* world, Entity* entity, float timestep) {
	/*
    printf("Y: %.2f | VY: %.2f | Grounded: %s\n",
           entity->position[1],
           entity->velocity[1],
           entity->is_on_ground ? "true" : "false");
	*/

    // Apply gravity
	if(entity->is_on_ground && !entity->was_on_ground) {
		entity->velocity[1] = 0.0f;
	}

    if (!entity->is_on_ground) {
        entity->velocity[1] += GRAVITY * timestep;
    }

    // Apply XZ drag
    float drag = 0.1f;
    for (int axis = 0; axis < 3; axis += 2) { // x and z only
        float decel = drag * fabs(entity->velocity[axis]);
        if (entity->velocity[axis] > 0.0f) {
            entity->velocity[axis] -= decel;
            if (entity->velocity[axis] < 0.0f) entity->velocity[axis] = 0.0f;
        } else if (entity->velocity[axis] < 0.0f) {
            entity->velocity[axis] += decel;
            if (entity->velocity[axis] > 0.0f) entity->velocity[axis] = 0.0f;
        }
        if (fabs(entity->velocity[axis]) < 0.01f) entity->velocity[axis] = 0.0f;
    }

	// Kill tiny velocities to prevent jitter
	if (fabs(entity->velocity[0]) < 0.01f) entity->velocity[0] = 0.0f;
	if (fabs(entity->velocity[2]) < 0.01f) entity->velocity[2] = 0.0f;

    // Predict next position
    vec3 next_pos;
    vec3 scaled_vel;
    glm_vec3_scale(entity->velocity, timestep, scaled_vel);
    glm_vec3_add(entity->position, scaled_vel, next_pos);

	// Ground probe
    Entity probe = *entity;
    probe.position[1] -= GROUND_TOLERANCE;
    bool touching_ground = collides_with_voxels(world, &probe);

    // Y-axis collision (up and down)
    Entity temp = *entity;
    temp.position[1] = next_pos[1];
    bool y_collision = collides_with_voxels(world, &temp);

    if (y_collision) {
        if (entity->velocity[1] < 0.0f) {
            // Falling
            entity->velocity[1] = 0.0f;
            entity->is_on_ground = true;
        } else if (entity->velocity[1] > 0.0f) {
            // Rising into ceiling
            entity->velocity[1] = 0.0f;
        }
    } else {
        entity->position[1] = next_pos[1];
        entity->is_on_ground = touching_ground && entity->velocity[1] <= 0.0f;
    }

    entity->was_on_ground = entity->is_on_ground;

 
	vec3 old_pos;
	glm_vec3_copy(entity->position, old_pos);
	// x collision
	entity->position[0] = next_pos[0];
	if (collides_with_voxels(world, entity)) {
    	entity->position[0] = old_pos[0];    // rollback
    	entity->velocity[0] = 0.0f;          // stop dead
	}

	// z collision
	entity->position[2] = next_pos[2];
	if (collides_with_voxels(world, entity)) {
    	entity->position[2] = old_pos[2];    // rollback
    	entity->velocity[2] = 0.0f;          // stop dead
	}
}

