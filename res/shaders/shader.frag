#version 330 core 

in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D blockTexture;

void main() 
{ 
    FragColor = vec4(0.0, 0.0, 0.0, 1.0);
    vec4 texColor = texture(blockTexture, TexCoord);

    if(texColor.a < 0.1)
        discard;
        
    FragColor = texColor;
    // FragColor = texture(blockTexture, TexCoord); 
}
