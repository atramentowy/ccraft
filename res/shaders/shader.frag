#version 330 core 

in vec2 frag_uv;
in float frag_light;

out vec4 frag_color;

uniform sampler2D block_texture;

void main() 
{
    vec4 tex_color = texture(block_texture, frag_uv);
    float light = clamp(frag_light, 0.0, 1.0);
    if(tex_color.a < 0.1)
        discard;

    frag_color = vec4(tex_color.rgb * light, tex_color.a);
}
