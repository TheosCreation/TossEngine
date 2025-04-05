#pragma once
#include "Utils.h"
#include "Resource.h"
#include "Shader.h"
#include "Texture.h"
#include "Serializable.h"
#include <variant>

struct Texture2DBinding {
	TexturePtr texture = nullptr;       // The texture resource.
	uint slot = 0;                // Texture unit/slot.
};

struct TextureCubeMapBinding {
	TextureCubeMapPtr texture = nullptr;       // The texture resource.
	uint slot = 0;                // Texture unit/slot.
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

class TOSSENGINE_API Material : public Resource, public Serializable
{
public:
	Material(ShaderPtr shader, const std::string& uniqueID, ResourceManager* manager);
	~Material();

    json serialize() const override;
    void deserialize(const json& data) override;

    void OnInspectorGUI() override;
    bool Delete(bool deleteSelf = true) override;

	void SetShader(const ShaderPtr& shader);
	ShaderPtr GetShader();

	bool Bind() const;

    TextureCubeMapPtr GetBinding(const string& bindingName);
private:
    void UpdateBindings();

	ShaderPtr m_shader;
	std::unordered_map<std::string, UniformValue> m_uniformValues;
};