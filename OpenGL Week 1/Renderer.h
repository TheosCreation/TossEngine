#pragma once
#include "Component.h"

class Renderer : public Component
{
public:
	Renderer();
	~Renderer();

	virtual void Render(UniformData data, RenderingPath renderPath) {}
	void SetMaterial(const MaterialPtr& material);

protected:
	MaterialPtr m_material;
};

