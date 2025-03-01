#pragma once
#include "Component.h"
#include "Material.h"

class Renderer : public Component
{
public:
	virtual void Render(UniformData data, RenderingPath renderPath) {}
	void SetMaterial(const MaterialPtr& material);

protected:
	MaterialPtr m_material;
};

