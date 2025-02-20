/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2024 Media Design School
File Name : QuadEntity.h
Description : Entity type that renders a quad
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
**/

#pragma once
#include "GraphicsEntity.h"
#include "Rect.h"

/**
 * @class QuadEntity
 * @brief Represents an entity that renders a quad shape in the graphics scene.
 *
 * The QuadEntity inherits from the GraphicsEntity class and is responsible for
 * rendering a quad mesh, updating its vertices, and handling shader data.
 */
class QuadEntity : public GraphicsEntity
{
public:
    /**
     * @brief Called when the quad entity is created.
     * This method should initialize the quad's properties and resources.
     */
    virtual void onCreate();

    /**
     * @brief Updates the vertices of the quad based on the given size.
     * @param size The new size for the quad as a Vector2.
     */
    void updateVertices(Vector2 size);

    /**
     * @brief Sets the uniform data for the shader.
     * @param data The uniform data to set for the shader.
     */
    virtual void setUniformData(UniformData data);

    /**
     * @brief Sets the shader for the quad entity.
     * @param shader A shared pointer to the shader to be used for rendering.
     */
    void setShader(const ShaderPtr& shader);

    /**
     * @brief Called every frame to update the graphics for the quad entity.
     * @param data The uniform data to set during the graphics update.
     */
    virtual void onGraphicsUpdate(UniformData data) override;

    /**
     * @brief Updates the graphics for the quad entity using new uniform data.
     * @param _data The new uniform data to use for updating.
     */
    void onGraphicsUpdate(NewUniformData& _data);

    /**
     * @brief Updates the graphics for the quad entity with additional texture data.
     * @param _data The new uniform data to use for updating.
     * @param _textureData The texture data to be applied to the quad.
     */
    void onGraphicsUpdate(NewUniformData& _data, NewExtraTextureData& _textureData);

    /**
     * @brief Sets the texture of the quad entity from a texture ID.
     * @param textureId The ID of the texture to be applied to the quad.
     */
    void setTextureFromId(uint textureId);

protected:
    VertexArrayObjectPtr m_mesh; // A shared pointer to the vertex array object representing the quad mesh.
};