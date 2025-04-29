/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2025 Media Design School
File Name : GeometryBuffer.h
Description : Manages a geometry buffer for deferred rendering.
              Stores position, normal, albedo, and other attributes as textures.
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
***/

#pragma once

#include "Utils.h"

class Shader;

/**
 * @class GeometryBuffer
 * @brief Singleton class representing a geometry buffer used in deferred rendering.
 *        Stores position, normal, albedo, and other surface properties in separate textures.
 */
class TOSSENGINE_API GeometryBuffer
{
public:
    /**
     * @brief Retrieves the singleton instance of the GeometryBuffer.
     * @return Reference to the GeometryBuffer instance.
     */
    static GeometryBuffer& GetInstance()
    {
        static GeometryBuffer instance;
        return instance;
    }

    // Delete copy constructor and assignment operator to enforce singleton behavior.
    GeometryBuffer(const GeometryBuffer&) = delete;
    GeometryBuffer& operator=(const GeometryBuffer&) = delete;

    /**
     * @brief Initializes the geometry buffer with a given window size.
     * @param _windowSize The dimensions of the buffer (usually viewport or screen size).
     */
    void Init(Vector2 _windowSize);

    /**
     * @brief Binds the geometry buffer for rendering.
     */
    void Bind();

    /**
     * @brief Unbinds the geometry buffer and returns to the default framebuffer.
     */
    void UnBind();

    /**
     * @brief Enables writing to the geometry buffer's depth buffer.
     * @param writeBuffer Optional attachment index to write to (default is 0).
     */
    void WriteDepth(uint writeBuffer = 0);

    /**
     * @brief Populates a shader with uniform data and G-buffer bindings.
     * @param _shader Shader to populate (should match G-buffer layout).
     */
    void PopulateShader(ShaderPtr _shader);

    /**
     * @brief Resizes the geometry buffer to a new screen/window size.
     * @param _windowSize The new dimensions for the buffer.
     */
    void Resize(Vector2 _windowSize);

private:
    /**
     * @brief Private constructor to enforce singleton pattern.
     */
    GeometryBuffer() = default;

    /**
     * @brief Private destructor.
     */
    ~GeometryBuffer() = default;

private:
    bool isInitilized = false; //!< Tracks whether the buffer has been initialized.
    Vector2 m_size = Vector2(0.0f); //!< Current dimensions of the buffer.

    uint FBO = 0; //!< Framebuffer Object ID.

    // G-buffer texture attachments:
    uint Texture_Position = 0;       //!< World-space position texture.
    uint Texture_Normal = 1;         //!< World-space normal texture.
    uint Texture_AlbedoShininess = 2; //!< Albedo + shininess (specular) texture.
    uint Texture_Depth = 3;          //!< Depth texture.
    uint Texture_Reflectivity = 4;   //!< Reflectivity (or custom material) texture.
};
