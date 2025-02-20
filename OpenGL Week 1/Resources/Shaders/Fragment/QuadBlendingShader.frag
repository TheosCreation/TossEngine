#version 460 core

in vec2 FragTexcoord;
in vec3 FragPos;

uniform sampler2D Texture0;
uniform sampler2D Texture1;
uniform sampler2D Texture2;
uniform sampler2D Texture3;
uniform sampler2D HeightMap; // Height map for blending

// Out
out vec4 FinalColor;

void main()
{
    // Sample both textures
    vec4 color0 = texture(Texture0, FragTexcoord);
    vec4 color1 = texture(Texture1, FragTexcoord);
    
    // Sample the height map to get the blending factor
    float blendFactor = texture(HeightMap, FragTexcoord).r; // Use the red channel for blending

    // Perform linear interpolation between Texture0 and Texture1
    FinalColor = mix(color0, color1, blendFactor);
}