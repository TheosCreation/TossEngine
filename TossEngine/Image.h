#pragma once
#include "UiElement.h"

class TOSSENGINE_API Image : public UiElement
{
public:
	Image() = default;
	~Image() = default;

	virtual void onCreate() override;

	virtual void Render(UniformData data, RenderingPath renderPath) override;

    virtual void OnInspectorGUI() override
    {
        // Display the material from the base Renderer component.
        UiElement::OnInspectorGUI();

        // Display the image's size.
        if (ImGui::DragFloat2("Size", m_size.Data(), 0.1f))
        {
            SetSize(m_size);
        }

        ResourceAssignableField(m_texture, "Texture");

        ImGui::Checkbox("Is Ui", &m_isUi);
    }

	void SetSize(Vector2 size);

	void SetTexture(const Texture2DPtr& texture);

    bool GetIsUi() const { return m_isUi; }

    Rect getWorldRect();

    Vector2 GetPivotOffsetFromCenter() const;


private:
	void updateVertices();

	VertexArrayObjectPtr m_vao;
	Vector2 m_size = { 1, 1 };
	Texture2DPtr m_texture;
    bool m_isUi = true;

    SERIALIZABLE_MEMBERS(m_size, m_material, m_texture, m_isUi, m_pivotPoint, m_anchorPoint)
};

REGISTER_COMPONENT(Image);