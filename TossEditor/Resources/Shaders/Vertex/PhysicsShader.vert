#version 460 core

layout(location = 0) in vec3 aPos;
uniform mat4 VPMatrix;

void main()
{
    gl_Position = VPMatrix * vec4(aPos, 1.0);
}
