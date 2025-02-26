#pragma once
#include "Utils.h"
#include "Resource.h"

class Material : public Resource
{
public:
	Material(const MaterialDesc& desc, const std::string& uniqueID, ResourceManager* manager);
	~Material();

	void SetShader(const ShaderPtr& shader);
	ShaderPtr GetShader();

private:
	ShaderPtr m_shader;
};