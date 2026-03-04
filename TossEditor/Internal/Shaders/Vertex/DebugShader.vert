#version 460 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aColor;

uniform mat4 VPMatrix;

out vec3 vColor;

void main()
{
    gl_Position = VPMatrix * vec4(aPosition, 1.0f);
    vColor = aColor;
}
