/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2024 Media Design School
File Name : ShadowMap.h
Description : ShadowMap class handles the creation and management of shadow maps for rendering shadows in a 3D scene.
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
 * @brief A class that represents a shadow map texture used for shadow rendering.
 */
class ShadowMap : public Texture
{
public:
    /**
     * @brief Constructor for the ShadowMap class.
     * @param _resolution The resolution of the shadow map.
     */
    ShadowMap(Vector2 _resolution);

    /**
     * @brief Destructor for the ShadowMap class.
     */
    ~ShadowMap();

    /**
     * @brief Binds the shadow map for rendering.
     * This prepares the shadow map for use in rendering shadows.
     */
    virtual void Bind();

    /**
     * @brief Unbinds the shadow map.
     * This releases the shadow map from the current context.
     */
    virtual void UnBind();

private:
    uint FBO; // Framebuffer Object for the shadow map.
    Vector2 m_resolution = Vector2(0); // Resolution of the shadow map.
    GLint m_prevViewport[4]; // Original viewport size before binding
};