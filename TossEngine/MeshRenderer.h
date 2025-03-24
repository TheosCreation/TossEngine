#pragma once
#include "Renderer.h"
#include "Mesh.h"

class TOSSENGINE_API MeshRenderer : public Renderer
{
public:
	MeshRenderer() = default;
	~MeshRenderer() = default;

    // Serialize the MeshRenderer to JSON
    json serialize() const override;

    // Deserialize the MeshRenderer from JSON
	void deserialize(const json& data) override;

	virtual void OnInspectorGUI() override
	{
		// Display the material from the base Renderer component.
		Renderer::OnInspectorGUI();

        ResourceSerializedField(m_mesh, "Mesh");
        ResourceSerializedField(m_shader, "Shader");
        ResourceSerializedField(m_geometryShader, "Geometry Shader");
        ResourceSerializedField(m_shadowShader, "Shadow Shader");
        ResourceSerializedField(m_texture, "Texture");
        ResourceSerializedField(m_reflectiveMap, "Reflective Map");

		ImGui::DragFloat("Shininess", &m_shininess, 0.1f);
		ImGui::DragFloat("Aplha", &m_alpha, 0.1f, 0.0f, 1.0f);
		ImGui::ColorEdit3("Color", &m_color.x);
	}

	void onShadowPass(uint index);

	void Render(UniformData data, RenderingPath renderPath) override;

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

REGISTER_COMPONENT(MeshRenderer);