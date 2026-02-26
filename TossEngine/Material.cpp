#include "Material.h"
#include "GraphicsEngine.h"
#include "TextureCubeMap.h"

Material::Material(ShaderPtr shader, const std::string& uniqueID, ResourceManager* manager) : Resource("", uniqueID, manager)
{
	m_shader = shader;
    UpdateBindings();
}

Material::Material(const std::string& uid, ResourceManager* mgr) : Resource(uid, mgr)
{
}

void Material::onCreateLate()
{
    if (!m_path.empty())
    {
        json j;
        std::ifstream file(m_path);
        if (file.is_open())
        {
            try
            {
                file >> j;
                deserialize(j);
            }
            catch (...)
            {
            }
        }
    }

    if (m_shader)
    {
        UpdateBindings();
    }

    isLoaded = true;
}

json Material::serialize() const
{
    json data;

    if (m_shader)
        data["shader"] = m_shader->getUniqueID();

    json bindingsJson;

    for (const auto& [name, value] : m_uniformValues)
    {
        std::visit([&](const auto& v)
            {
                using T = std::decay_t<decltype(v)>;

                if constexpr (std::is_same_v<T, float>)
                {
                    bindingsJson[name] = v;
                }
                else if constexpr (std::is_same_v<T, Vector2>)
                {
                    bindingsJson[name] = { v.x, v.y };
                }
                else if constexpr (std::is_same_v<T, Vector3>)
                {
                    bindingsJson[name] = { v.x, v.y, v.z };
                }
                else if constexpr (std::is_same_v<T, Vector4>)
                {
                    bindingsJson[name] = { v.x, v.y, v.z, v.w };
                }
                else if constexpr (std::is_same_v<T, Texture2DBinding>)
                {
                    if (v.texture)
                    {
                        bindingsJson[name] = {
                            { "type", "Texture2D" },
                            { "id", v.texture->getUniqueID() },
                            { "slot", v.slot }
                        };
                    }
                }
                else if constexpr (std::is_same_v<T, TextureCubeMapBinding>)
                {
                    if (v.texture)
                    {
                        bindingsJson[name] = {
                            { "type", "CubeMap" },
                            { "id", v.texture->getUniqueID() },
                            { "slot", v.slot }
                        };
                    }
                }
                // Do not serialize Mat4
            }, value);
    }

    data["bindings"] = bindingsJson;

    return data;
}

void Material::deserialize(const json& data)
{
    auto& resourceManager = ResourceManager::GetInstance();

    if (data.contains("shader"))
    {
        std::string shaderId = data["shader"];
        m_shader = resourceManager.get<Shader>(shaderId);
        UpdateBindings(); // Must call to prepare the uniform map
    }

    if (!m_shader || !data.contains("bindings")) return;

    const json& bindingsJson = data["bindings"];

    for (auto& [name, value] : bindingsJson.items())
    {
        if (!m_uniformValues.contains(name)) continue; // Skip if not bound

        auto& uniform = m_uniformValues[name];

        std::visit([&](auto& existingValue)
            {
                using T = std::decay_t<decltype(existingValue)>;

                if constexpr (std::is_same_v<T, float>)
                {
                    if (value.is_number()) existingValue = value.get<float>();
                }
                else if constexpr (std::is_same_v<T, Vector2>)
                {
                    if (value.is_array() && value.size() == 2)
                        existingValue = Vector2(value[0], value[1]);
                }
                else if constexpr (std::is_same_v<T, Vector3>)
                {
                    if (value.is_array() && value.size() == 3)
                        existingValue = Vector3(value[0], value[1], value[2]);
                }
                else if constexpr (std::is_same_v<T, Vector4>)
                {
                    if (value.is_array() && value.size() == 4)
                        existingValue = Vector4(value[0], value[1], value[2], value[3]);
                }
                else if constexpr (std::is_same_v<T, Texture2DBinding>)
                {
                    if (value.contains("id"))
                    {
                        existingValue.texture = resourceManager.get<Texture2D>(value["id"]);
                        if (value.contains("slot"))
                            existingValue.slot = value["slot"];
                    }
                }
                else if constexpr (std::is_same_v<T, TextureCubeMapBinding>)
                {
                    if (value.contains("id"))
                    {
                        existingValue.texture = resourceManager.get<TextureCubeMap>(value["id"]);
                        if (value.contains("slot"))
                            existingValue.slot = value["slot"];
                    }
                }

            }, uniform);
    }


    isLoaded = true;
}

void Material::OnInspectorGUI()
{
    ImGui::Text(("Material Inspector - ID: " + m_uniqueID).c_str());
    ImGui::Separator();

    if (ResourceDropdownField(m_shader, "Shader"))
    {
        UpdateBindings();
    }

    if (!m_shader) return;

    ImGui::Text("Bindings:");
    ImGui::Separator();

    for (auto& [uniformName, value] : m_uniformValues)
    {
        ImGui::PushID(uniformName.c_str()); // Ensure unique ID

        std::visit([&](auto&& arg) mutable
            {
                using T = std::decay_t<decltype(arg)>;

                if constexpr (std::is_same_v<T, float>)
                {
                    ImGui::DragFloat(uniformName.c_str(), &arg, 0.01f);
                    value = arg;
                }
                else if constexpr (std::is_same_v<T, Vector2>)
                {
                    float arr[2] = { arg.x, arg.y };
                    if (ImGui::DragFloat2(uniformName.c_str(), arr, 0.01f))
                    {
                        value = Vector2(arr[0], arr[1]);
                    }
                }
                else if constexpr (std::is_same_v<T, Vector3>)
                {
                    float arr[3] = { arg.x, arg.y, arg.z };
                    if (ImGui::ColorEdit3(uniformName.c_str(), arr)) // You can also use DragFloat3 if you prefer
                    {
                        value = Vector3(arr[0], arr[1], arr[2]);
                    }
                }
                else if constexpr (std::is_same_v<T, Vector4>)
                {
                    float arr[4] = { arg.x, arg.y, arg.z, arg.w };
                    if (ImGui::ColorEdit4(uniformName.c_str(), arr)) // Color picker with alpha
                    {
                        value = Vector4(arr[0], arr[1], arr[2], arr[3]);
                    }
                }
                else if constexpr (std::is_same_v<T, Texture2DBinding>)
                {
                    ResourceDropdownField(arg.texture, uniformName + "Texture2D");
                    value = arg;
                }
                else if constexpr (std::is_same_v<T, TextureCubeMapBinding>)
                {
                    ResourceDropdownField(arg.texture, uniformName + "CubeMap");
                    value = arg;
                }
                // Mat4 could be editable with a matrix editor if you want to go advanced
                else if constexpr (std::is_same_v<T, Mat4>)
                {
                    ImGui::Text("%s: [Matrix4x4]", uniformName.c_str());
                    // Optional: Custom editor
                }

            }, value);

        ImGui::PopID();
    }
}

bool Material::Delete(bool deleteSelf)
{
    return false;
}

void Material::SetShader(const ShaderPtr& shader)
{
	m_shader = shader;
}

ShaderPtr Material::GetShader()
{
	return m_shader;
}

bool Material::Bind() const
{
    if (!m_shader)
        return false;


    auto& graphicsEngine = GraphicsEngine::GetInstance();
    graphicsEngine.setShader(m_shader);
    bool failed = false;
    // Iterate over all uniform values stored in the material.
    for (const auto& [uniformName, value] : m_uniformValues)
    {
        std::visit([&](auto&& arg)
            {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, int>)
                    m_shader->setInt(uniformName, arg);
                else if constexpr (std::is_same_v<T, float>)
                    m_shader->setFloat(uniformName, arg);
                else if constexpr (std::is_same_v<T, Vector2>)
                    m_shader->setVec2(uniformName, arg);
                else if constexpr (std::is_same_v<T, Vector3>)
                    m_shader->setVec3(uniformName, arg);
                else if constexpr (std::is_same_v<T, Vector4>)
                    m_shader->setVec4(uniformName, arg);
                else if constexpr (std::is_same_v<T, Mat4>)
                    m_shader->setMat4(uniformName, arg);
                else if constexpr (std::is_same_v<T, Texture2DBinding>)
                    if (arg.texture != nullptr)
                    {
                        m_shader->setTexture2D(arg.texture, arg.slot, uniformName);
                    }
                    else
                    {
                        failed = true;
                    }
                else if constexpr (std::is_same_v<T, TextureCubeMapBinding>)
                    if (arg.texture != nullptr)
                    {
                        m_shader->setTextureCubeMap(arg.texture, arg.slot, uniformName);
                    }
                    else
                    {
                        failed = true;
                    }
            }, value);
    }

    return !failed;
}

TextureCubeMapPtr Material::GetBinding(const string& bindingName)
{
    // Check if the bindingName exists in the m_uniformValues container
    TextureCubeMapPtr cubemapTexture;
    for (const auto& [uniformName, value] : m_uniformValues)
    {
        std::visit([&](auto&& arg)
            {
                using T = std::decay_t<decltype(arg)>; //making T be type of the argument
                if constexpr (std::is_same_v<T, TextureCubeMapBinding>)
                    if (arg.texture != nullptr)
                    {
                        cubemapTexture = arg.texture;
                    }
            }, value);
    }

    return cubemapTexture;
}

Texture2DPtr Material::GetTexture2DBinding(const std::string& bindingName)
{
    // Check if the bindingName exists in the m_uniformValues container
    Texture2DPtr texture2D;
    for (const auto& [uniformName, value] : m_uniformValues)
    {
        std::visit([&](auto&& arg)
            {
                using T = std::decay_t<decltype(arg)>; //making T be type of the argument
                if constexpr (std::is_same_v<T, Texture2DBinding>)
                    if (arg.texture != nullptr)
                    {
                        texture2D = arg.texture;
                    }
            }, value);
    }

    return texture2D;
}

void Material::UpdateBindings()
{
    if (!m_shader)
    {
        return;
    }

    std::unordered_map<std::string, UniformValue> oldValues = m_uniformValues;
    m_uniformValues.clear();

    GLuint programId = m_shader->getId();

    for (auto& binding : m_shader->getBindings())
    {
        const std::string& uniformName = binding.name;

        GLint location = glGetUniformLocation(programId, uniformName.c_str());
        if (location < 0)
        {
            continue;
        }

        auto oldIt = oldValues.find(uniformName);

        switch (binding.type)
        {
            case GL_FLOAT:
            {
                float value = 0.0f;
                if (oldIt != oldValues.end() && std::holds_alternative<float>(oldIt->second))
                {
                    value = std::get<float>(oldIt->second);
                }
                m_uniformValues[uniformName] = value;
                break;
            }

            case GL_FLOAT_VEC2:
            {
                Vector2 value = Vector2{};
                if (oldIt != oldValues.end() && std::holds_alternative<Vector2>(oldIt->second))
                {
                    value = std::get<Vector2>(oldIt->second);
                }
                m_uniformValues[uniformName] = value;
                break;
            }

            case GL_FLOAT_VEC3:
            {
                Vector3 value = Vector3{};
                if (oldIt != oldValues.end() && std::holds_alternative<Vector3>(oldIt->second))
                {
                    value = std::get<Vector3>(oldIt->second);
                }
                m_uniformValues[uniformName] = value;
                break;
            }

            case GL_FLOAT_VEC4:
            {
                Vector4 value = Vector4{};
                if (oldIt != oldValues.end() && std::holds_alternative<Vector4>(oldIt->second))
                {
                    value = std::get<Vector4>(oldIt->second);
                }
                m_uniformValues[uniformName] = value;
                break;
            }

            case GL_FLOAT_MAT4:
            {
                Mat4 value = Mat4{ 1.0f };
                if (oldIt != oldValues.end() && std::holds_alternative<Mat4>(oldIt->second))
                {
                    value = std::get<Mat4>(oldIt->second);
                }
                m_uniformValues[uniformName] = value;
                break;
            }

            case GL_SAMPLER_2D:
            {
                Texture2DBinding bindingValue;
                bindingValue.texture = nullptr;
                bindingValue.slot = 0;

                if (oldIt != oldValues.end() && std::holds_alternative<Texture2DBinding>(oldIt->second))
                {
                    bindingValue = std::get<Texture2DBinding>(oldIt->second);
                }

                m_uniformValues[uniformName] = bindingValue;
                break;
            }

            case GL_SAMPLER_CUBE:
            {
                TextureCubeMapBinding bindingValue;
                bindingValue.texture = nullptr;
                bindingValue.slot = 0;

                if (oldIt != oldValues.end() && std::holds_alternative<TextureCubeMapBinding>(oldIt->second))
                {
                    bindingValue = std::get<TextureCubeMapBinding>(oldIt->second);
                }

                m_uniformValues[uniformName] = bindingValue;
                break;
            }

            default:
            {
                break;
            }
        }
    }
}
