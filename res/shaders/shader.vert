#version 330 core
layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec2 in_uv;
layout (location = 2) in float in_light;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out vec2 frag_uv;
out float frag_light;

void main()
{
    frag_uv = in_uv;
    frag_light = clamp(in_light, 0.0, 1.0);
    gl_Position = projection * view * model * vec4(in_pos, 1.0);
}
