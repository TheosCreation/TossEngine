#version 460 core

layout(location = 0) out vec4 Texture_Position;
layout(location = 1) out vec4 Texture_Normal;
layout(location = 2) out vec4 Texture_AlbedoShininess;

in vec2 FragTexcoord;
in vec3 FragNormal;
in vec3 FragPos;
in float Height;

layout(binding = 0) uniform sampler2D Texture0; // Base texture
layout(binding = 1) uniform sampler2D Texture1; // Additional texture
layout(binding = 2) uniform sampler2D Texture2; // Additional texture
layout(binding = 3) uniform sampler2D Texture3; // Additional texture
layout(binding = 4) uniform sampler2D HeightMap; // Height map for blending

uniform float ObjectShininess = 32.0f;

const float lowerHeight = 30;
const float lowerMiddleHeight = 60;
const float upperMiddleHeight = 90;
const float upperHeight = 120;


void main()
{
    // Sample textures
    vec4 BaseTexture = texture(Texture0, FragTexcoord);
    vec4 AdditionalTexture1 = texture(Texture1, FragTexcoord); // Additional texture for blending
    vec4 AdditionalTexture2 = texture(Texture2, FragTexcoord);
    vec4 AdditionalTexture3 = texture(Texture3, FragTexcoord);

    float blendGrass = 1.0 - smoothstep(lowerHeight, lowerMiddleHeight, Height);
    float blendDirt = smoothstep(lowerHeight, lowerMiddleHeight, Height) * (1.0 - smoothstep(lowerMiddleHeight, upperMiddleHeight, Height));
    float blendStone = smoothstep(lowerMiddleHeight, upperMiddleHeight, Height) * (1.0 - smoothstep(upperMiddleHeight, upperHeight, Height));
    float blendSnow = smoothstep(upperMiddleHeight, upperHeight, Height);
    
    vec4 BlendedTexture = BaseTexture * blendGrass + AdditionalTexture1 * blendDirt + AdditionalTexture2 * blendStone + AdditionalTexture3 * blendSnow;
	
    Texture_Position = vec4(FragPos, 1.0f);
	Texture_Normal = vec4(FragNormal, 1.0f);
	Texture_AlbedoShininess.rgb = BlendedTexture.rgb;
	Texture_AlbedoShininess.a = ObjectShininess;
}