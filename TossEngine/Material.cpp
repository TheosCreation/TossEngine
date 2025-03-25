#include "Material.h"
#include "GraphicsEngine.h"

Material::Material(ShaderPtr shader, const std::string& uniqueID, ResourceManager* manager) : Resource("", uniqueID, manager)
{
	m_shader = shader;

    for (const auto& binding : m_shader->getBindings().uniformBindings)
    {
        switch (binding.type)
        {
        case GL_FLOAT:
            m_uniformValues[binding.name] = 0.0f;
            break;
        case GL_FLOAT_VEC2:
            m_uniformValues[binding.name] = Vector2(0.0f, 0.0f);
            break;
        case GL_FLOAT_VEC3:
            m_uniformValues[binding.name] = Vector3(0.0f, 0.0f, 0.0f);
            break;
        case GL_FLOAT_VEC4:
            m_uniformValues[binding.name] = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
            break;
        case GL_FLOAT_MAT4:
            m_uniformValues[binding.name] = Mat4(1.0f); // Identity matrix.
            break;
        case GL_SAMPLER_2D:
        {
            Texture2DBinding tb;
            tb.texture = nullptr;
            tb.slot = 0;
            m_uniformValues[binding.name] = tb;
        }
        break;
        case GL_SAMPLER_CUBE:
        {
            TextureCubeMapBinding tb;
            tb.texture = nullptr;
            tb.slot = 0;
            m_uniformValues[binding.name] = tb;
        }
        break;
        default:
            break;
        }
    }
}

Material::~Material()
{
}

void Material::OnInspectorGUI()
{
    ImGui::Text(("Material Inspector - ID: " + m_uniqueID).c_str());
    ImGui::Separator();
    m_shader->OnInspectorGUI();
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

void Material::Bind() const
{
    if (!m_shader)
        return;


    auto& graphicsEngine = GraphicsEngine::GetInstance();
    graphicsEngine.setShader(m_shader);

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
                    m_shader->setTexture2D(arg.texture, arg.slot, uniformName);
                else if constexpr (std::is_same_v<T, TextureCubeMapBinding>)
                    m_shader->setTextureCubeMap(arg.texture, arg.slot, uniformName);
            }, value);
    }
}
