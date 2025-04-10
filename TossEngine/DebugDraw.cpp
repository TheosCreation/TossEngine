#include "DebugDraw.h"
#include "ResourceManager.h"
#include "Shader.h"

ShaderPtr DebugDraw::GetShader()
{
    if (m_debugShader == nullptr)
    {
        ShaderDesc debugShaderDesc = { "Internal/Shaders/DebugShader.vert", "Internal/Shaders/DebugShader.frag" };
        m_debugShader = ResourceManager::GetInstance().createShader(debugShaderDesc, "DebugShader");
    }
    return m_debugShader;
}
