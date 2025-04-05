#pragma once
#include "Component.h"
#include "Material.h"

class TOSSENGINE_API Renderer : public Component
{
public:
	virtual void Render(UniformData data, RenderingPath renderPath) {}
	void SetMaterial(const MaterialPtr& material);
    MaterialPtr GetMaterial() const;

	virtual void OnInspectorGUI() override 
	{
        ResourceAssignableField(m_material, "Material");
	}

protected:
	MaterialPtr m_material;
};

