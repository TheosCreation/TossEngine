#version 460 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec4 velocity;
layout(location = 2) in vec4 Color; 

uniform mat4 VPMatrix;

out vec4 FragColor;

void main()
{
    gl_Position = VPMatrix * vec4(vertexPosition.xyz, 1.0f);
    FragColor = Color;

    float alpha = 1.0 - (length(velocity.xyz) / 20.0f);
    FragColor.a *= alpha;
}