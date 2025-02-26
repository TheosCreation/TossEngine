#version 460 core

in vec2 FragTexcoord;

uniform sampler2D Texture0;

// Out
out vec4 FinalColor;

void main()
{
    FinalColor = texture(Texture0, FragTexcoord);
}