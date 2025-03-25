#version 460 core

layout(location = 0) out vec4 Texture_Position;
layout(location = 1) out vec4 Texture_Normal;
layout(location = 2) out vec4 Texture_AlbedoShininess;
layout(location = 3) out vec4 Texture_Reflectivity;

in vec2 FragTexcoord;
in vec3 FragNormal;
in vec3 FragPos;

uniform sampler2D Texture0; // Albedo texture
uniform sampler2D Texture_ReflectivityMap; // Reflectivity texture (optional)
uniform float ObjectShininess = 32.0f;
uniform float ObjectReflectivity = 1.0f; // Default reflectivity value
uniform vec3 uColor;
uniform bool useTexture;

void main()
{
    // Write fragment position to G-buffer
    Texture_Position = vec4(FragPos, 1.0f);

    // Write fragment normal to G-buffer
    Texture_Normal = vec4(normalize(FragNormal), 1.0f);
    
    vec3 Albedo = useTexture ? texture(Texture0, FragTexcoord).rgb : uColor;
    // Write albedo and shininess to G-buffer
    Texture_AlbedoShininess = vec4(Albedo, ObjectShininess);

    // Write reflectivity to G-buffer
    //float Reflectivity = ObjectReflectivity; // Default value
    //if (textureSize(Texture_ReflectivityMap, 0).x > 1) // Check if reflectivity texture is valid
    //{
    //    Reflectivity = texture(Texture_ReflectivityMap, FragTexcoord).r; // Sample reflectivity from texture
    //}
    Texture_Reflectivity = vec4(ObjectReflectivity, 0.0, 0.0, 1.0); // Store reflectivity in the red channel
}