#pragma once
#include "Component.h"
#include "Material.h"

class TOSSENGINE_API Renderer : public Component
{
public:
	virtual void Render(UniformData data, RenderingPath renderPath) {}
	void SetMaterial(const MaterialPtr& material);

	virtual void OnInspectorGUI() override 
	{
        if (ImGui::CollapsingHeader("Material", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (m_material)
            {
                // Display the material's name.
                ImGui::Text("Material: ");
                // If Material has its own inspector, you could call:
                // m_material->OnInspectorGUI();
            }
            else
            {
                ImGui::Text("No Material assigned.");
            }
        }
	}

protected:
	MaterialPtr m_material;
};

