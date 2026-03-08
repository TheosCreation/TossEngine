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
inline void to_json(json& j, const Texture2DBinding& b)
{
    j = json::object();
    j["slot"] = b.slot;
    j["texture"] = b.texture; // uses your existing to_json(Texture2DPtr)
}

inline void from_json(const json& j, Texture2DBinding& b)
{
    b.slot = 0;
    b.texture = nullptr;

    if (j.contains("slot"))
    {
        b.slot = j.at("slot").get<uint>();
    }
    if (j.contains("texture"))
    {
        b.texture = j.at("texture").get<Texture2DPtr>();
    }
}

inline void to_json(json& j, const TextureCubeMapBinding& b)
{
    j = json::object();
    j["slot"] = b.slot;
    j["texture"] = b.texture; // needs to_json(TextureCubeMapPtr)
}

inline void from_json(const json& j, TextureCubeMapBinding& b)
{
    b.slot = 0;
    b.texture = nullptr;

    if (j.contains("slot"))
    {
        b.slot = j.at("slot").get<uint>();
    }
    if (j.contains("texture"))
    {
        b.texture = j.at("texture").get<TextureCubeMapPtr>();
    }
}

static const char* UniformType_Int = "int";
static const char* UniformType_Float = "float";
static const char* UniformType_Vec2 = "vec2";
static const char* UniformType_Vec3 = "vec3";
static const char* UniformType_Vec4 = "vec4";
static const char* UniformType_Mat4 = "mat4";
static const char* UniformType_Tex2D = "tex2d";
static const char* UniformType_TexCube = "texcube";

inline void to_json(json& j, const UniformValue& value)
{
    j = json::object();

    if (std::holds_alternative<int>(value))
    {
        j["t"] = UniformType_Int;
        j["v"] = std::get<int>(value);
        return;
    }

    if (std::holds_alternative<float>(value))
    {
        j["t"] = UniformType_Float;
        j["v"] = std::get<float>(value);
        return;
    }

    if (std::holds_alternative<Vector2>(value))
    {
        j["t"] = UniformType_Vec2;
        j["v"] = std::get<Vector2>(value);
        return;
    }

    if (std::holds_alternative<Vector3>(value))
    {
        j["t"] = UniformType_Vec3;
        j["v"] = std::get<Vector3>(value);
        return;
    }

    if (std::holds_alternative<Vector4>(value))
    {
        j["t"] = UniformType_Vec4;
        j["v"] = std::get<Vector4>(value);
        return;
    }

    if (std::holds_alternative<Mat4>(value))
    {
        j["t"] = UniformType_Mat4;
        j["v"] = std::get<Mat4>(value);
        return;
    }

    if (std::holds_alternative<Texture2DBinding>(value))
    {
        j["t"] = UniformType_Tex2D;
        j["v"] = std::get<Texture2DBinding>(value);
        return;
    }

    if (std::holds_alternative<TextureCubeMapBinding>(value))
    {
        j["t"] = UniformType_TexCube;
        j["v"] = std::get<TextureCubeMapBinding>(value);
        return;
    }

    j["t"] = "unknown";
    j["v"] = nullptr;
}

inline void from_json(const json& j, UniformValue& value)
{
    if (!j.is_object())
    {
        value = 0;
        return;
    }

    std::string t = "";
    if (j.contains("t"))
    {
        t = j.at("t").get<std::string>();
    }

    if (!j.contains("v"))
    {
        value = 0;
        return;
    }

    const json& v = j.at("v");

    if (t == UniformType_Int)
    {
        value = v.get<int>();
        return;
    }

    if (t == UniformType_Float)
    {
        value = v.get<float>();
        return;
    }

    if (t == UniformType_Vec2)
    {
        value = v.get<Vector2>();
        return;
    }

    if (t == UniformType_Vec3)
    {
        value = v.get<Vector3>();
        return;
    }

    if (t == UniformType_Vec4)
    {
        value = v.get<Vector4>();
        return;
    }

    if (t == UniformType_Mat4)
    {
        value = v.get<Mat4>();
        return;
    }

    if (t == UniformType_Tex2D)
    {
        value = v.get<Texture2DBinding>();
        return;
    }

    if (t == UniformType_TexCube)
    {
        value = v.get<TextureCubeMapBinding>();
        return;
    }

    value = 0;
}

// -------------------------
// unordered_map<string, UniformValue> JSON
// Stored as object of key -> UniformValue
// -------------------------

namespace nlohmann
{
    template <>
    struct adl_serializer<std::unordered_map<std::string, UniformValue>>
    {
        static void to_json(json& j, const std::unordered_map<std::string, UniformValue>& map)
        {
            j = json::object();
            for (const auto& kv : map)
            {
                j[kv.first] = kv.second;
            }
        }

        static void from_json(const json& j, std::unordered_map<std::string, UniformValue>& map)
        {
            map.clear();

            if (!j.is_object())
            {
                return;
            }

            for (auto it = j.begin(); it != j.end(); ++it)
            {
                const std::string key = it.key();
                const json& val = it.value();

                UniformValue parsedValue = val.get<UniformValue>();
                map.emplace(key, parsedValue);
            }
        }
    };
}

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

    void onCreateLate() override;
    
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
    
    void setBinding(const std::string& uniformName, const Texture2DPtr& texture, uint slot = 0);
    void setBinding(const std::string& uniformName, const TextureCubeMapPtr& texture, uint slot = 0);
    void setBinding(const std::string& uniformName, float value);
    void setBinding(const std::string& uniformName, int value);
    void setBinding(const std::string& uniformName, const Vector2& value);
    void setBinding(const std::string& uniformName, const Vector3& value);
    void setBinding(const std::string& uniformName, const Vector4& value);
    void setBinding(const std::string& uniformName, const Mat4& value);
    
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
    Texture2DPtr GetTexture2DBinding(const std::string& bindingName);
    float GetFloatBinding(const std::string& bindingName);

    std::string GetAssetSaveExtension() const override
    {
        return ".mat";
    }
    
    std::vector<std::string> GetImportExtensions() const override
    {
        return { GetAssetSaveExtension() };
    }
    
private:
    /**
     * @brief Updates internal uniform bindings after shader or resource changes.
     */
    void UpdateBindings();

    ShaderPtr m_shader; //!< Shader used by the material.
    std::unordered_map<std::string, UniformValue> m_uniformValues; //!< Map of uniform values bound to the material.
    
    SERIALIZABLE_MEMBERS(m_shader, m_path, m_uniformValues) // TODO: rework this to use this
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