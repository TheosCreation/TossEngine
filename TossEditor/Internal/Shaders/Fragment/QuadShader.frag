#version 460 core

in vec2 FragTexcoord;

uniform sampler2D Texture0;
uniform vec3 color;

// Out
out vec4 FinalColor;

void main()
{
    FinalColor = vec4(color, 1.0) * texture(Texture0, FragTexcoord);
}