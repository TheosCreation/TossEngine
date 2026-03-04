#version 460 core

layout(location = 0) in vec3 Position;

uniform mat4 VPMatrix;

out vec3 FragTexcoord;

void main()
{
    vec4 Pos = VPMatrix * vec4(Position, 1.0f);
    gl_Position = Pos.xyww;
    FragTexcoord = vec3(Position.xyz);
}