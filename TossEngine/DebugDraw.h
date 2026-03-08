/***
DeviousDevs
Auckland
New Zealand
(c) 2026 DeviousDevs
File Name : DebugDraw.h
Description : Provides access to debug rendering resources, such as shaders, used for visualizing debug elements in TossEngine.
Author : Theo Morris
Mail : theo.morris@outlook.co.nz
***/

#pragma once

#include "Utils.h"

/**
 * @class DebugDraw
 * @brief Singleton class responsible for managing debug rendering resources such as debug shaders.
 */
class TOSSENGINE_API DebugDraw
{
public:
    /**
     * @brief Gets the singleton instance of DebugDraw.
     * @return Reference to the DebugDraw instance.
     */
    static DebugDraw& GetInstance()
    {
        static DebugDraw instance;
        return instance;
    }

    /**
     * @brief Returns the shader used for rendering debug primitives.
     * @return Shared pointer to the debug shader.
     */
    ShaderPtr GetShader();

    /**
     * @brief Cleans up resources used by DebugDraw.
     */
    void CleanUp();

private:
    DebugDraw() = default;  //!< Private constructor (singleton pattern).
    ~DebugDraw() = default; //!< Private destructor.

private:
    ShaderPtr m_debugShader = nullptr; //!< Shader used for debug rendering.
};