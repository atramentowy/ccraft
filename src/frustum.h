#ifndef FRUSTUM_H
#define FRUSTUM_H

#include <stdbool.h>
#include <cglm/cglm.h>
#include <math.h>
#include <string.h>

#include "camera.h"
#include "chunk.h"

typedef struct {
    vec3 normal;
    float distance;
} Plane;

typedef struct {
    Plane left_face;
    Plane right_face;
    Plane bottom_face;
    Plane top_face;
    Plane near_face;
    Plane far_face;
} Frustum;

static inline Frustum create_frustum_from_camera(
    const Camera* cam,
    float aspect,
    float fov_y,
    float z_near,
    float z_far
) {
    Frustum frustum;

    vec3 forward, up, right;
    glm_vec3_copy((float*)cam->front, forward); glm_vec3_normalize(forward);
    glm_vec3_copy((float*)cam->up, up);         glm_vec3_normalize(up);
    glm_vec3_copy((float*)cam->right, right);   glm_vec3_normalize(right);

    vec3 cam_pos;
    glm_vec3_copy((float*)cam->position, cam_pos);

    vec3 near_center, far_center;
    glm_vec3_scale(forward, z_near, near_center);
    glm_vec3_add(cam_pos, near_center, near_center);

    glm_vec3_scale(forward, z_far, far_center);
    glm_vec3_add(cam_pos, far_center, far_center);

    float tan_fov = tanf(fov_y * 0.5f);
    float near_height = z_near * tan_fov;
    float near_width  = near_height * aspect;
    float far_height  = z_far * tan_fov;
    float far_width   = far_height * aspect;

    // near plane
    glm_vec3_copy(forward, frustum.near_face.normal);
    glm_vec3_normalize(frustum.near_face.normal);
    frustum.near_face.distance = glm_vec3_dot(frustum.near_face.normal, near_center);

    // far plane
    vec3 neg_forward;
    glm_vec3_negate_to(forward, neg_forward);
    glm_vec3_copy(neg_forward, frustum.far_face.normal);
    glm_vec3_normalize(frustum.far_face.normal);
    frustum.far_face.distance = glm_vec3_dot(frustum.far_face.normal, far_center);

    // right plane
    {
        vec3 far_right, point;
        glm_vec3_scale(right, far_width, far_right);
        glm_vec3_sub(far_center, far_right, point);

        vec3 dir;
        glm_vec3_sub(point, cam_pos, dir);
        glm_vec3_cross(dir, up, frustum.right_face.normal);
        glm_vec3_normalize(frustum.right_face.normal);
        frustum.right_face.distance = glm_vec3_dot(frustum.right_face.normal, point);
    }

    // left plane
    {
        vec3 far_left, point;
        glm_vec3_scale(right, far_width, far_left);
        glm_vec3_add(far_center, far_left, point);

        vec3 dir;
        glm_vec3_sub(point, cam_pos, dir);
        glm_vec3_cross(up, dir, frustum.left_face.normal);
        glm_vec3_normalize(frustum.left_face.normal);
        frustum.left_face.distance = glm_vec3_dot(frustum.left_face.normal, point);
    }

    // top plane
    {
        vec3 far_top, point;
        glm_vec3_scale(up, far_height, far_top);
        glm_vec3_sub(far_center, far_top, point);

        vec3 dir;
        glm_vec3_sub(point, cam_pos, dir);
        glm_vec3_cross(right, dir, frustum.top_face.normal);
        glm_vec3_normalize(frustum.top_face.normal);
        frustum.top_face.distance = glm_vec3_dot(frustum.top_face.normal, point);
    }

	// bottom plane
    {
        vec3 far_bottom, point;
        glm_vec3_scale(up, far_height, far_bottom);
        glm_vec3_add(far_center, far_bottom, point);

        vec3 dir;
        glm_vec3_sub(point, cam_pos, dir);
        glm_vec3_cross(dir, right, frustum.bottom_face.normal);
        glm_vec3_normalize(frustum.bottom_face.normal);
        frustum.bottom_face.distance = glm_vec3_dot(frustum.bottom_face.normal, point);
    }

    return frustum;
}

static inline bool chunk_in_frustum(const Frustum* f, int cx, int cy, int cz) {	
    vec3 box_min = {
        cx * CHUNK_SIZE,
        cy * CHUNK_SIZE,
        cz * CHUNK_SIZE
    };
    vec3 box_max = {
        box_min[0] + CHUNK_SIZE,
        box_min[1] + CHUNK_SIZE,
        box_min[2] + CHUNK_SIZE
    };

    const Plane* planes[6] = {
        &f->left_face,   
		&f->right_face,
        &f->bottom_face, 
		&f->top_face,
        &f->near_face,   
		&f->far_face
    };

    for (int i = 0; i < 6; ++i) {
        const Plane* p = planes[i];

        vec3 positive;
        positive[0] = (p->normal[0] >= 0.0f) ? box_max[0] : box_min[0];
        positive[1] = (p->normal[1] >= 0.0f) ? box_max[1] : box_min[1];
        positive[2] = (p->normal[2] >= 0.0f) ? box_max[2] : box_min[2];

        float dist = glm_vec3_dot((float*)p->normal, positive) - p->distance;

        if (dist < 0.0f) {
			// printf("chunk culled");
            return false;
        }
    }
    return true;
}

#endif // FRUSTRUM_H
