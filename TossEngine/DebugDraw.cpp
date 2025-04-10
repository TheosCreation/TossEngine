#include "DebugDraw.h"
#include "ResourceManager.h"
#include "Shader.h"

ShaderPtr DebugDraw::GetShader()
{
    if (m_debugShader == nullptr)
    {
        ShaderDesc debugShaderDesc = { "Internal/DebugShader.vert", "Internal/DebugShader.frag" };
        m_debugShader = ResourceManager::GetInstance().createShader(debugShaderDesc, "DebugShader");
    }
    return m_debugShader;
}
