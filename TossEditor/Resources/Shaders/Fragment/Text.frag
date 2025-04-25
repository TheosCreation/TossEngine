#version 460 core

in vec2 FragTexcoord;

uniform sampler2D text;
uniform vec3 textColor;

// Out
out vec4 FinalColor;

void main()
{
    vec4 sampledText = vec4(1.0, 1.0, 1.0, texture(text, FragTexcoord).r);
    FinalColor = vec4(textColor, 1.0) * sampledText;
}