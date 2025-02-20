#version 460 core

in vec2 FragTexcoord;

uniform sampler2D Texture_Position;     // G-buffer: Fragment position
uniform sampler2D Texture_Normal;       // G-buffer: Fragment normal
uniform sampler2D Texture_AlbedoShininess; // G-buffer: Albedo and shininess
uniform sampler2D Texture0;             // Base texture

// Light parameters
uniform vec3 CameraPos;

// Output
out vec4 FinalColor;

void main()
{
    // Sample textures
    vec3 FragPos = texture(Texture_Position, FragTexcoord).xyz;
    vec3 FragNormal = normalize(texture(Texture_Normal, FragTexcoord).xyz);
    vec3 FragAlbedo = texture(Texture_AlbedoShininess, FragTexcoord).rgb;
    float ObjectShininess = texture(Texture_AlbedoShininess, FragTexcoord).a;
    vec3 BaseColor = texture(Texture0, FragTexcoord).rgb;

    // Combine the results
    vec3 Color = FragAlbedo;

    // Output final color
    FinalColor = vec4(Color, 1.0);
}
