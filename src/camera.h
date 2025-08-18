#ifndef CAMERA_H
#define CAMERA_H

#include <cglm/cglm.h>
#include <stdbool.h>
#include <math.h>

// Camera movement directions
typedef enum {
    CAMERA_FORWARD,
    CAMERA_BACKWARD,
    CAMERA_LEFT,
    CAMERA_RIGHT
} CameraMovementDir;

// Default camera values
#define CAMERA_YAW         -90.0f
#define CAMERA_PITCH        0.0f
#define CAMERA_SPEED        2.5f
#define CAMERA_SENSITIVITY  0.1f
#define CAMERA_ZOOM         45.0f

typedef struct {
    vec3 position;
    vec3 front;
    vec3 up;
    vec3 right;
    vec3 world_up;

    float yaw;
    float pitch;

    float movement_speed;
    float mouse_sensitivity;
    float zoom;
} Camera;

// Internal helper to update camera vectors
static inline void update_camera_vectors(Camera *cam) {
    vec3 front;
    front[0] = cosf(glm_rad(cam->yaw)) * cosf(glm_rad(cam->pitch));
    front[1] = sinf(glm_rad(cam->pitch));
    front[2] = sinf(glm_rad(cam->yaw)) * cosf(glm_rad(cam->pitch));
    glm_normalize_to(front, cam->front);

    glm_cross(cam->front, cam->world_up, cam->right);
    glm_normalize(cam->right);

    glm_cross(cam->right, cam->front, cam->up);
    glm_normalize(cam->up);
}

// Initialize camera with position, up vector, yaw and pitch
static inline void camera_init(Camera *cam, vec3 position, vec3 up, float yaw, float pitch) {
    glm_vec3_copy(position, cam->position);
    glm_vec3_copy(up, cam->world_up);
    cam->yaw = yaw;
    cam->pitch = pitch;
    cam->movement_speed = CAMERA_SPEED;
    cam->mouse_sensitivity = CAMERA_SENSITIVITY;
    cam->zoom = CAMERA_ZOOM;
    glm_vec3_copy((vec3){0.0f, 0.0f, -1.0f}, cam->front);
    update_camera_vectors(cam);
}

// Get view matrix from camera
static inline void camera_get_view_matrix(Camera *cam, mat4 view) {
    vec3 center;
    glm_vec3_add(cam->position, cam->front, center);
    glm_lookat(cam->position, center, cam->up, view);
}

// Process keyboard input
static inline void camera_process_keyboard(Camera *cam, CameraMovementDir direction, float delta_time) {
    float velocity = cam->movement_speed * delta_time;
    vec3 offset;

    switch (direction) {
        case CAMERA_FORWARD:
            glm_vec3_scale(cam->front, velocity, offset);
            glm_vec3_add(cam->position, offset, cam->position);
            break;
        case CAMERA_BACKWARD:
            glm_vec3_scale(cam->front, velocity, offset);
            glm_vec3_sub(cam->position, offset, cam->position);
            break;
        case CAMERA_LEFT:
            glm_vec3_scale(cam->right, velocity, offset);
            glm_vec3_sub(cam->position, offset, cam->position);
            break;
        case CAMERA_RIGHT:
            glm_vec3_scale(cam->right, velocity, offset);
            glm_vec3_add(cam->position, offset, cam->position);
            break;
    }
}

// Process mouse movement
static inline void camera_process_mouse(Camera *cam, float xoffset, float yoffset, bool constrain_pitch) {
    xoffset *= cam->mouse_sensitivity;
    yoffset *= cam->mouse_sensitivity;

    cam->yaw += xoffset;
    cam->pitch += yoffset;

    if (constrain_pitch) {
        if (cam->pitch > 89.0f)
            cam->pitch = 89.0f;
        if (cam->pitch < -89.0f)
            cam->pitch = -89.0f;
    }

    update_camera_vectors(cam);
}

// Process mouse scroll
static inline void camera_process_scroll(Camera *cam, float yoffset) {
    cam->zoom -= yoffset;
    if (cam->zoom < 1.0f)
        cam->zoom = 1.0f;
    if (cam->zoom > 45.0f)
        cam->zoom = 45.0f;
}

#endif // CAMERA_H
