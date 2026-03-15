#version 460 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec2 vertexTexCoords;
layout(location = 2) in vec3 vertexNormal;
layout(location = 3) in ivec4 vertexBoneIndices;
layout(location = 4) in vec4 vertexBoneWeights;
layout(location = 5) in mat4 InstancedModel;

uniform mat4 modelMatrix;
uniform mat4 VPMatrix;

uniform bool u_IsSkinned;
uniform bool u_IsInstanced;
uniform mat4 u_BoneMatrices[128];

out vec2 FragTexcoord;
out vec3 FragNormal;
out vec3 FragPos;

void main()
{
    mat4 skinMatrix = mat4(1.0f);

    if (u_IsSkinned)
    {
        skinMatrix =
            u_BoneMatrices[vertexBoneIndices.x] * vertexBoneWeights.x +
            u_BoneMatrices[vertexBoneIndices.y] * vertexBoneWeights.y +
            u_BoneMatrices[vertexBoneIndices.z] * vertexBoneWeights.z +
            u_BoneMatrices[vertexBoneIndices.w] * vertexBoneWeights.w;
    }

    vec4 localPosition = skinMatrix * vec4(vertexPosition, 1.0f);
    vec3 localNormal = mat3(skinMatrix) * vertexNormal;

    mat4 finalModelMatrix = modelMatrix;
    if (u_IsInstanced)
    {
        finalModelMatrix = modelMatrix * InstancedModel;
    }

    vec4 worldPosition = finalModelMatrix * localPosition;

    gl_Position = VPMatrix * worldPosition;
    FragTexcoord = vertexTexCoords;
    FragNormal = mat3(transpose(inverse(finalModelMatrix))) * localNormal;
    FragPos = vec3(worldPosition);
}