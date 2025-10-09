#include "debug.h"

#include <GLFW/glfw3.h>

void debug_set_wireframe_mode(bool value) {
    if(value) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void debug_set_backface_culling(bool value) {
    if (value) {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
    } else {
        glDisable(GL_CULL_FACE);
    }
}

