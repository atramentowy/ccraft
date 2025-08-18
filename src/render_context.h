#ifndef RENDER_CONTEXT_H
#define RENDER_CONTEXT_H

#include <cglm/cglm.h>
#include "frustum.h"

typedef struct {
	Frustum frustum;
    mat4 projection;
    mat4 view;
} RenderContext;

#endif
