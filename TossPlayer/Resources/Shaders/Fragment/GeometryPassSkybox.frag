#version 460 core

layout(location = 0) out vec4 Texture_Position;          // G-buffer: Fragment position
layout(location = 1) out vec4 Texture_Normal;            // G-buffer: Fragment normal
layout(location = 2) out vec4 Texture_AlbedoShininess;   // G-buffer: Albedo and shininess

in vec3 FragTexcoord; // Input from vertex shader (texture coordinates for the skybox)

layout(binding = 6) uniform samplerCube Texture_Skybox; // Skybox texture

void main()
{
    // Skybox doesn't require actual position and normal for lighting
    Texture_Position = vec4(0.0, 0.0, 0.0, 1.0); // Default position (can also set to world position if needed)
    Texture_Normal = vec4(0.0, 0.0, 1.0, 0.0);   // Default normal pointing up, change if needed

    // Set the albedo color and a default shininess value
    Texture_AlbedoShininess.rgb = texture(Texture_Skybox, FragTexcoord).rgb; // Set the RGB from the skybox color
    Texture_AlbedoShininess.a = 1.0f;            // Default shininess value
}
