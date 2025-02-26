#include "Material.h"

Material::Material(const MaterialDesc& desc, const std::string& uniqueID, ResourceManager* manager) : Resource("", uniqueID, manager)
{
	m_shader = desc.shader;
}

Material::~Material()
{
}

void Material::SetShader(const ShaderPtr& shader)
{
	m_shader = shader;
}

ShaderPtr Material::GetShader()
{
	return m_shader;
}
