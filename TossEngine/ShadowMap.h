/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2024 Media Design School
File Name : ShadowMap.h
Description : ShadowMap class handles creation and management of depth textures for real-time shadow rendering.
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
***/

#pragma once
#include "Utils.h"
#include "Texture.h"
#include <glew.h>
#include <glfw3.h>

/**
 * @class ShadowMap
 * @brief Represents a depth texture used for rendering real-time shadows.
 */
class TOSSENGINE_API ShadowMap : public Texture
{
public:
    /**
     * @brief Constructor.
     * @param resolution Resolution of the shadow map (width and height).
     */
    ShadowMap(Vector2 resolution);

    /**
     * @brief Destructor.
     */
    ~ShadowMap();

    /**
     * @brief Binds the shadow map framebuffer for writing depth information.
     */
    void Bind();

    /**
     * @brief Unbinds the shadow map framebuffer and restores the previous viewport.
     */
    void UnBind();

private:
    uint FBO = 0;              //!< Framebuffer Object ID used for depth rendering.
    Vector2 m_resolution = {}; //!< Shadow map resolution.
    GLint m_prevViewport[4] = { 0, 0, 0, 0 }; //!< Previous OpenGL viewport settings (restored after unbinding).
};
