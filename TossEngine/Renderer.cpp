#include "Renderer.h"

void Renderer::SetMaterial(const MaterialPtr& material)
{
	m_material = material;
}

MaterialPtr Renderer::GetMaterial() const
{
	return m_material;
}
