#pragma once
#include "Renderer.h"

class Image : public Renderer
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
        ImGui::DragFloat2("Size", &m_size.x, 0.1f);

        // Display the texture, assuming m_texture has a GetName() method.
        if (m_texture)
        {
            ImGui::Text("Texture: ");
        }
        else
        {
            ImGui::Text("No Texture assigned.");
        }
    }

	void SetSize(Vector2 size);

	void SetTexture(const TexturePtr& texture);


private:
	void updateVertices();

	VertexArrayObjectPtr m_vbo;
	Vector2 m_size = { 1, 1 };
	TexturePtr m_texture;
};

