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

        ResourceAssignableField(m_mesh, "Mesh");
        ResourceAssignableField(m_shader, "Shader");
        ResourceAssignableField(m_geometryShader, "Geometry Shader");
        ResourceAssignableField(m_shadowShader, "Shadow Shader");
        ResourceAssignableField(m_texture, "Texture");
        ResourceAssignableField(m_reflectiveMap, "Reflective Map");

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

	void SetTexture(Texture2DPtr texture);

	void SetReflectiveMapTexture(Texture2DPtr texture);

	void SetShininess(float shininess);

	void SetAlpha(float transparency);
	float GetAlpha();

	void SetColor(Vector3 color);

private:
	MeshPtr m_mesh;

	ShaderPtr m_shader;
	ShaderPtr m_geometryShader;
	ShaderPtr m_shadowShader;

	Texture2DPtr m_texture;
    Texture2DPtr m_reflectiveMap;

	float m_shininess = 32.0f;
	float m_alpha = 1.0f;
	Vector3 m_color = Color::Purple;
};

REGISTER_COMPONENT(MeshRenderer);