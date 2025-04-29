/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2025 Media Design School
File Name : Material.h
Description : A material resource that manages shaders and uniform bindings for rendering.
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
***/

#pragma once

#include "Utils.h"
#include "Resource.h"
#include "Shader.h"
#include "Texture2D.h"
#include "TextureCubeMap.h"
#include "Serializable.h"
#include <variant>

/**
 * @struct Texture2DBinding
 * @brief Binding information for a 2D texture in a shader.
 */
struct Texture2DBinding {
    Texture2DPtr texture = nullptr; //!< The texture resource.
    uint slot = 0;                  //!< Texture unit (binding slot).
};

/**
 * @struct TextureCubeMapBinding
 * @brief Binding information for a cube map texture in a shader.
 */
struct TextureCubeMapBinding {
    TextureCubeMapPtr texture = nullptr; //!< The cube map resource.
    uint slot = 0;                       //!< Texture unit (binding slot).
};

/**
 * @typedef UniformValue
 * @brief A variant type that stores possible uniform types for a material.
 */
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

/**
 * @class Material
 * @brief A resource representing material properties including shader and texture bindings.
 */
class TOSSENGINE_API Material : public Resource
{
public:
    /**
     * @brief Constructor for creating a new material with a shader.
     * @param shader The shader to assign.
     * @param uniqueID The unique resource ID.
     * @param manager Pointer to the resource manager.
     */
    Material(ShaderPtr shader, const std::string& uniqueID, ResourceManager* manager);

    /**
     * @brief Constructor for loading an existing material.
     * @param uid The unique resource ID.
     * @param mgr Pointer to the resource manager.
     */
    Material(const std::string& uid, ResourceManager* mgr);

    /**
     * @brief Serializes the material to JSON.
     * @return JSON object representing the material.
     */
    json serialize() const override;

    /**
     * @brief Deserializes the material from JSON.
     * @param data The JSON object.
     */
    void deserialize(const json& data) override;

    /**
     * @brief Draws the inspector UI for editing the material.
     */
    void OnInspectorGUI() override;

    /**
     * @brief Deletes the material.
     * @param deleteSelf Whether to delete the resource from the manager.
     * @return True if deleted, false otherwise.
     */
    bool Delete(bool deleteSelf = true) override;

    /**
     * @brief Sets the shader associated with this material.
     * @param shader The shader to set.
     */
    void SetShader(const ShaderPtr& shader);

    /**
     * @brief Gets the shader associated with this material.
     * @return Shared pointer to the shader.
     */
    ShaderPtr GetShader();

    /**
     * @brief Binds the material's shader and uniforms for rendering.
     * @return True if successful, false otherwise.
     */
    bool Bind() const;

    /**
     * @brief Retrieves a bound cube map texture by binding name.
     * @param bindingName The name of the binding.
     * @return Shared pointer to the cube map texture, or nullptr if not found.
     */
    TextureCubeMapPtr GetBinding(const std::string& bindingName);

private:
    /**
     * @brief Updates internal uniform bindings after shader or resource changes.
     */
    void UpdateBindings();

    ShaderPtr m_shader; //!< Shader used by the material.
    std::unordered_map<std::string, UniformValue> m_uniformValues; //!< Map of uniform values bound to the material.
};

REGISTER_RESOURCE(Material)

// --- JSON Serialization ---

inline void to_json(json& j, const MaterialPtr& material) {
    if (material)
    {
        j = json{ { "id", material->getUniqueID() } };
    }
    else {
        j = nullptr;
    }
}

inline void from_json(const json& j, MaterialPtr& material) {
    if (j.contains("id") && !j["id"].is_null())
        material = ResourceManager::GetInstance().get<Material>(j["id"].get<std::string>());
}