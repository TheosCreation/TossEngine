#version 460 core
#define MAX_POINT_LIGHTS 4
#define MAX_DIR_LIGHTS 2
#include "Shadows.glsl"
#include "Lighting.glsl"

in vec3 FragPos;
in vec2 FragTexcoord;
in vec3 FragNormal;

uniform sampler2D Texture0;
uniform samplerCube Texture_Skybox;
uniform sampler2D ReflectionMap;

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
uniform float ObjectShininess = 32.0f;

// Out
out vec4 FinalColor;

void main()
{
    // Normalize the normal and calculate view and reflection directions
    vec3 Normal = normalize(FragNormal);
    vec3 ViewDir = normalize(FragPos - CameraPos);
    vec3 ReflectDir = reflect(ViewDir, Normal);

    // Ambient Component
    vec3 Ambient = AmbientStrength * AmbientColor;

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
    
    vec3 LightShadow = Ambient + TotalLightOutput;

    // Sample textures
    vec4 ObjectTexture = texture(Texture0, FragTexcoord);
    vec4 ReflectionTexture = texture(Texture_Skybox, ReflectDir);
    float Reflectivity = texture(ReflectionMap, FragTexcoord).r; // Sample the red channel of the reflection map

    // Mix object texture and reflection texture based on reflectivity
    vec4 MixedTexture = mix(ObjectTexture, ReflectionTexture, Reflectivity);

    // Calculate the final color
    FinalColor = vec4(LightShadow, 1.0) * MixedTexture;
}