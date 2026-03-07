#pragma once
#include "Renderer.h"
#include "Mesh.h"

class TOSSENGINE_API MeshRenderer : public Renderer
{
public:
	MeshRenderer() = default;
	~MeshRenderer() = default;

    void onCreateLate() override;

	virtual void OnInspectorGUI() override
	{
		// Display the material from the base Renderer component.
		Renderer::OnInspectorGUI();

        ResourceAssignableField(m_mesh, "Mesh");
	}

	void onShadowPass(uint index);

	void Render(UniformData data, RenderingPath renderPath) override;

	void SetMesh(MeshPtr mesh);
	MeshPtr GetMesh() const;

    float GetAlpha() const;
    Vector3 GetExtent() override;

private:
	MeshPtr m_mesh;

	ShaderPtr m_geometryShader;
	ShaderPtr m_shadowShader;

    SERIALIZABLE_MEMBERS(m_mesh, m_material)
};

REGISTER_COMPONENT(MeshRenderer);
