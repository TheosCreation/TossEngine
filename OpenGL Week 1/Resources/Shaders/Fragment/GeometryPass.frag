#version 460 core

layout(location = 0) out vec4 Texture_Position;
layout(location = 1) out vec4 Texture_Normal;
layout(location = 2) out vec4 Texture_AlbedoShininess;

in vec2 FragTexcoord;
in vec3 FragNormal;
in vec3 FragPos;

uniform sampler2D Texture0;

uniform float ObjectShininess = 32.0f;

void main()
{
	Texture_Position = vec4(FragPos, 1.0f);
	Texture_Normal = vec4(normalize(FragNormal), 1.0f);
	Texture_AlbedoShininess.rgb = texture(Texture0, FragTexcoord).rgb;
	Texture_AlbedoShininess.a = ObjectShininess;
}