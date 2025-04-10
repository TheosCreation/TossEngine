#pragma once
#include "Utils.h"

class TOSSENGINE_API DebugDraw
{
public:
    static DebugDraw& GetInstance()
    {
        static DebugDraw instance;
        return instance;
    }

    ShaderPtr GetShader();

private:
    DebugDraw() = default;
    ~DebugDraw() = default;

    ShaderPtr m_debugShader = nullptr;
};