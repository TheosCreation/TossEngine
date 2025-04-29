#version 460 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec2 vertexTexCoords;

out vec2 FragTexcoord;

void main()
{
    gl_Position = vec4(vertexPosition, 1.0f);
    FragTexcoord = vertexTexCoords;
}