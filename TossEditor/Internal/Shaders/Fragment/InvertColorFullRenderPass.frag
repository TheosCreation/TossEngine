#version 460 core

in vec2 FragTexcoord;

uniform sampler2D Texture0;

// Out
out vec4 FinalColor;

void main()
{
    // Sample the color from the texture
    vec4 color = texture(Texture0, FragTexcoord);
    
    // Invert the color by subtracting from 1.0
    FinalColor = vec4(1.0 - color.rgb, color.a);
}