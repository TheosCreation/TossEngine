#version 460 core

layout(location = 0) in vec3 vertexPosition;

uniform mat4 modelMatrix;
uniform mat4 VPLight;

void main()
{
    gl_Position = VPLight * modelMatrix * vec4(vertexPosition, 1.0f);
}