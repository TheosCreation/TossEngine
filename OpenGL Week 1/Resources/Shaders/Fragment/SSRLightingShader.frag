#version 460 core
#define MAX_POINT_LIGHTS 20
#define MAX_DIR_LIGHTS 2
#include "Shadows.glsl"
#include "Lighting.glsl"

in vec2 FragTexcoord;

uniform sampler2D Texture_Position;         // G-buffer: Fragment position
uniform sampler2D Texture_Normal;           // G-buffer: Fragment normal
uniform sampler2D Texture_AlbedoShininess;  // G-buffer: Albedo and shininess
uniform sampler2D Texture_Depth;            // G-buffer: Depth
uniform sampler2D Texture_Reflectivity;     // G-buffer: Reflectivity
uniform samplerCube Texture_Skybox;         // Skybox Texture: Reflectivity

// Shadow maps and light space matrices
uniform sampler2D Texture_ShadowMap[MAX_DIR_LIGHTS]; // Array for shadow maps
uniform mat4 VPLight[MAX_DIR_LIGHTS]; // Array for light-space matrices

uniform float AmbientStrength = 0.15f;
uniform vec3 AmbientColor = vec3(1.0f, 1.0f, 1.0f);

uniform PointLight PointLightArray[MAX_POINT_LIGHTS];
uniform uint PointLightCount;

uniform DirectionalLight DirLightArray[MAX_DIR_LIGHTS];
uniform uint DirectionalLightCount;

uniform SpotLight SpotLight1;
uniform int SpotLightStatus;

uniform vec3 CameraPos;

// Output
out vec4 FinalColor;

void main()
{
    // Sample textures
    vec3 FragPos = texture(Texture_Position, FragTexcoord).xyz;
    vec3 FragNormal = texture(Texture_Normal, FragTexcoord).xyz;
    vec3 FragAlbedo = texture(Texture_AlbedoShininess, FragTexcoord).rgb;
    float ObjectShininess = texture(Texture_AlbedoShininess, FragTexcoord).a;

    vec3 ViewDir = normalize(FragPos - CameraPos);
    vec3 Normal = normalize(FragNormal);
    vec3 ReflectDir = reflect(ViewDir, Normal);

    // Ambient Component
    vec3 Ambient = AmbientColor * AmbientStrength;

    // Calculate lighting
    vec3 TotalLightOutput = vec3(0.0);
    for (uint i = 0; i < PointLightCount; ++i)
    {
        TotalLightOutput += CalculatePointLight(PointLightArray[i], ViewDir, ObjectShininess, Normal, FragPos);
    }

    for (uint i = 0; i < DirectionalLightCount; ++i)
    {
        float Shadow = CalculateShadow(VPLight[i], Texture_ShadowMap[i], FragPos);
        TotalLightOutput += (1.0 - Shadow) * CalculateDirectionalLight(DirLightArray[i], ViewDir, ObjectShininess, Normal);
    }

    if (SpotLightStatus == 1)
    {
        TotalLightOutput += CalculateSpotLight(SpotLight1, ViewDir, ObjectShininess, Normal, FragPos);
    }

    vec3 Lighting = Ambient + TotalLightOutput;
    
    // Sample reflection texture from the skybox
    //vec4 ReflectionTexture = texture(Texture_Skybox, ReflectDir);
    //
    //// Sample reflectivity from the G-buffer
    //float Reflectivity = texture(Texture_Reflectivity, FragTexcoord).r;
    //
    //// Mix albedo and reflection based on reflectivity
    //vec3 MixedTexture = mix(FragAlbedo, ReflectionTexture.rgb, Reflectivity);

    // Output final color
    //FinalColor = vec4(ReflectionTexture.rgb, 1.0);
    FinalColor = vec4(Lighting * FragAlbedo, 1.0);
}
