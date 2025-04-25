#pragma once
#include "Renderer.h"

class TOSSENGINE_API Image : public Renderer
{
public:
	Image() = default;
	~Image() = default;

	virtual void onCreate() override;

	virtual void Render(UniformData data, RenderingPath renderPath) override;

    virtual void OnInspectorGUI() override
    {
        // Display the material from the base Renderer component.
        Renderer::OnInspectorGUI();

        // Display the image's size.
        if (ImGui::DragFloat2("Size", m_size.Data(), 0.1f))
        {
            SetSize(m_size);
        }

        ResourceAssignableField(m_texture, "Texture");
    }

	void SetSize(Vector2 size);

	void SetTexture(const TexturePtr& texture);


private:
	void updateVertices();

	VertexArrayObjectPtr m_vao;
	Vector2 m_size = { 1, 1 };
	TexturePtr m_texture;

    SERIALIZABLE_MEMBERS(m_size, m_material, m_texture)
};

REGISTER_COMPONENT(Image);