/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2024 Media Design School
File Name : GraphicsEntity.h
Description : Entity type to be rendered by the graphics engine
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
**/

#pragma once
#include "Entity.h"
#include "Rect.h"
#include "Texture.h"
#include "Shader.h"

// Forward declaration of EntitySystem class
class GameObjectManager;

/**
 * @class GraphicsEntity
 * @brief An entity type to be rendered by the graphics engine.
 */
class GraphicsEntity : public GameObject
{
public:
    /**
     * @brief Constructor for the GraphicsEntity class.
     */
    GraphicsEntity();

    /**
     * @brief Destructor for the GraphicsEntity class.
     */
    virtual ~GraphicsEntity();

    /**
     * @brief Called every frame to update the graphics.
     * @param deltaTime The time elapsed since the last update.
     */
    virtual void onGraphicsUpdate(UniformData data);
    virtual void onShadowPass(uint index);
    virtual void onGeometryPass(UniformData data);
    virtual void onLightingPass(UniformData data);

    /**
     * @brief Sets the uniform data for the shader.
     * @param data The uniform data to set.
     */
    virtual void setUniformData(UniformData data) {};

    /**
     * @brief Gets the shader used by this entity.
     * @return A shared pointer to the shader.
     */
    ShaderPtr getShader() const;

    /**
     * @brief Sets the shader to be used by this entity.
     * @param shader A shared pointer to the shader.
     */
    void setShader(const ShaderPtr& shader);

    void setShadowShader(const ShaderPtr& shader);
    void setGeometryShader(const ShaderPtr& shader);
    void setLightingShader(const ShaderPtr& shader);

    /**
     * @brief Gets the texture used by this entity.
     * @return A shared pointer to the texture.
     */
    TexturePtr getTexture() const;

    /**
     * @brief Sets the texture to be used by this entity.
     * @param texture A shared pointer to the texture.
     */
    void setTexture(const TexturePtr& texture);
    void setTexture1(const TexturePtr& texture);
    void setTexture2(const TexturePtr& texture);
    void setTexture3(const TexturePtr& texture);
    void setHeightMap(const TexturePtr& heightMapTexture);

    void setTransparency(float transparency);

    /**
     * @brief Sets the color to be used by this entity.
     * @param color A vec3 of rgb values.
     */
    void setColor(Vector3 color);
    Vector3 getColor();

protected:
    ShaderPtr m_shader = nullptr; //The shader used by this entity
    ShaderPtr m_shadowShader = nullptr; //The shader used for the shadow pass by this entity
    ShaderPtr m_geometryShader = nullptr; //The shader used for the geometry pass by this entity
    ShaderPtr m_lightingShader = nullptr; //The shader used for the lighting pass by this entity
    Vector3 m_color = Color::Red; //backup color or for solid colored objects
    TexturePtr m_texture = nullptr; //The texture used by this entity
    TexturePtr m_texture1 = nullptr;
    TexturePtr m_texture2 = nullptr;
    TexturePtr m_texture3 = nullptr;
    TexturePtr m_heightMap = nullptr;
    float m_transparentAlpha = 1.0f;

    void ProcessUniformData(const std::string& name, const std::any& value);
    void ProcessTextureData(const std::string& name, const std::tuple<TexturePtr, uint>& value);
};