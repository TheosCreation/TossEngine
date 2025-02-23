#pragma once
#include "Component.h"

class MeshRenderer : public Component
{
public:
	MeshRenderer();
	~MeshRenderer();

	void onShadowPass(uint index);

	void Render(UniformData data);

	void SetMesh(MeshPtr mesh);

	void SetShadowShader(ShaderPtr shader);

	void SetShader(ShaderPtr shader);

	void SetGeometryShader(ShaderPtr shader);

	void SetTexture(TexturePtr texture);

	void SetReflectiveMapTexture(TexturePtr texture);

	void SetShininess(float shininess);

	void SetAlpha(float transparency);
	float GetAlpha();

	void SetColor(Vector3 color);

private:
	MeshPtr m_mesh;

	ShaderPtr m_shader;
	ShaderPtr m_geometryShader;
	ShaderPtr m_shadowShader;

	TexturePtr m_texture;
	TexturePtr m_reflectiveMap;

	float m_shininess = 32.0f;
	float m_alpha = 1.0f;
	Vector3 m_color = Color::Purple;
};

