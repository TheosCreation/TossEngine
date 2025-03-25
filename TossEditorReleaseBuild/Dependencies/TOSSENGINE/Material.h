#pragma once
#include "Utils.h"
#include "Resource.h"
#include "Shader.h"
#include "Texture.h"
#include <variant>

struct Texture2DBinding {
	TexturePtr texture;       // The texture resource.
	uint slot;                // Texture unit/slot.
};

struct TextureCubeMapBinding {
	TexturePtr texture;       // The texture resource.
	uint slot;                // Texture unit/slot.
};

using UniformValue = std::variant<
	int,
	float,
	Vector2,
	Vector3,
	Vector4,
	Mat4,
	Texture2DBinding,
	TextureCubeMapBinding
>;

class TOSSENGINE_API Material : public Resource
{
public:
	Material(ShaderPtr shader, const std::string& uniqueID, ResourceManager* manager);
	~Material();

	void SetShader(const ShaderPtr& shader);
	ShaderPtr GetShader();

	void Bind() const;
private:
	ShaderPtr m_shader;
	std::unordered_map<std::string, UniformValue> m_uniformValues;
};