#include "Renderer.h"

Renderer::Renderer()
{
}

Renderer::~Renderer()
{
}

void Renderer::SetMaterial(const MaterialPtr& material)
{
	m_material = material;
}
