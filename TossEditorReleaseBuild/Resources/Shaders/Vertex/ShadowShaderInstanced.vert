#version 460 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 3) in mat4 InstancedModel;

uniform mat4 modelMatrix;
uniform mat4 VPLight;

void main()
{
    mat4 finalModel = modelMatrix * InstancedModel;
    gl_Position = VPLight * finalModel * vec4(vertexPosition, 1.0f);
}