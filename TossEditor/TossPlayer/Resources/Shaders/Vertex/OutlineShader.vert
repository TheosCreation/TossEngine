#version 460 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec2 vertexTexCoords;
layout(location = 2) in vec3 vertexNormal;

uniform mat4 modelMatrix;
uniform mat4 VPMatrix;

out vec2 FragTexcoord;
out vec3 FragNormal;
out vec3 FragPos;

void main()
{
    float normalShift = 1.1f;
    vec3 shiftedPosition = vertexPosition + (vertexNormal * normalShift);

    gl_Position = VPMatrix * modelMatrix * vec4(shiftedPosition, 1.0f);
    FragTexcoord = vertexTexCoords;
    FragNormal = mat3(transpose(inverse(modelMatrix))) * vertexNormal;
    FragPos = vec3(modelMatrix * vec4(shiftedPosition, 1.0f));
}
