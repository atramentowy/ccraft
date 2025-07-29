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
} Camera_Movement;

// Default camera values
#define CAMERA_YAW         -90.0f
#define CAMERA_PITCH        0.0f
#define CAMERA_SPEED        2.5f
#define CAMERA_SENSITIVITY  0.1f
#define CAMERA_ZOOM         45.0f

typedef struct {
    vec3 Position;
    vec3 Front;
    vec3 Up;
    vec3 Right;
    vec3 WorldUp;

    float Yaw;
    float Pitch;

    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;
} Camera;

// Internal helper to update camera vectors
static inline void update_camera_vectors(Camera *cam) {
    vec3 front;
    front[0] = cosf(glm_rad(cam->Yaw)) * cosf(glm_rad(cam->Pitch));
    front[1] = sinf(glm_rad(cam->Pitch));
    front[2] = sinf(glm_rad(cam->Yaw)) * cosf(glm_rad(cam->Pitch));
    glm_normalize_to(front, cam->Front);

    glm_cross(cam->Front, cam->WorldUp, cam->Right);
    glm_normalize(cam->Right);

    glm_cross(cam->Right, cam->Front, cam->Up);
    glm_normalize(cam->Up);
}

// Initialize camera with position, up vector, yaw and pitch
static inline void camera_init(Camera *cam, vec3 position, vec3 up, float yaw, float pitch) {
    glm_vec3_copy(position, cam->Position);
    glm_vec3_copy(up, cam->WorldUp);
    cam->Yaw = yaw;
    cam->Pitch = pitch;
    cam->MovementSpeed = CAMERA_SPEED;
    cam->MouseSensitivity = CAMERA_SENSITIVITY;
    cam->Zoom = CAMERA_ZOOM;
    glm_vec3_copy((vec3){0.0f, 0.0f, -1.0f}, cam->Front);
    update_camera_vectors(cam);
}

// Get view matrix from camera
static inline void camera_get_view_matrix(Camera *cam, mat4 view) {
    vec3 center;
    glm_vec3_add(cam->Position, cam->Front, center);
    glm_lookat(cam->Position, center, cam->Up, view);
}

// Process keyboard input
static inline void camera_process_keyboard(Camera *cam, Camera_Movement direction, float deltaTime) {
    float velocity = cam->MovementSpeed * deltaTime;
    vec3 offset;

    switch (direction) {
        case CAMERA_FORWARD:
            glm_vec3_scale(cam->Front, velocity, offset);
            glm_vec3_add(cam->Position, offset, cam->Position);
            break;
        case CAMERA_BACKWARD:
            glm_vec3_scale(cam->Front, velocity, offset);
            glm_vec3_sub(cam->Position, offset, cam->Position);
            break;
        case CAMERA_LEFT:
            glm_vec3_scale(cam->Right, velocity, offset);
            glm_vec3_sub(cam->Position, offset, cam->Position);
            break;
        case CAMERA_RIGHT:
            glm_vec3_scale(cam->Right, velocity, offset);
            glm_vec3_add(cam->Position, offset, cam->Position);
            break;
    }
}

// Process mouse movement
static inline void camera_process_mouse(Camera *cam, float xoffset, float yoffset, bool constrainPitch) {
    xoffset *= cam->MouseSensitivity;
    yoffset *= cam->MouseSensitivity;

    cam->Yaw += xoffset;
    cam->Pitch += yoffset;

    if (constrainPitch) {
        if (cam->Pitch > 89.0f)
            cam->Pitch = 89.0f;
        if (cam->Pitch < -89.0f)
            cam->Pitch = -89.0f;
    }

    update_camera_vectors(cam);
}

// Process mouse scroll
static inline void camera_process_scroll(Camera *cam, float yoffset) {
    cam->Zoom -= yoffset;
    if (cam->Zoom < 1.0f)
        cam->Zoom = 1.0f;
    if (cam->Zoom > 45.0f)
        cam->Zoom = 45.0f;
}

#endif // CAMERA_H
