/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2024 Media Design School
File Name : MeshEntity.h
Description : Entity type that renders meshes
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
**/

#pragma once
#include "GraphicsEntity.h"

/**
 * @class MeshEntity
 * @brief An entity type that renders meshes.
 */
class MeshEntity : public GraphicsEntity
{
public:
    /**
     * @brief Sets the mesh for this entity.
     * @param mesh A shared pointer to the mesh.
     */
    void setMesh(const MeshPtr& mesh);

    /**
     * @brief Gets the mesh for this entity.
     * @return A shared pointer to the mesh.
     */
    MeshPtr getMesh();

    /**
     * @brief Sets the reflective map texture to be used by this entity.
     * @param texture A shared pointer to the texture.
     */
    void setReflectiveMapTexture(const TexturePtr& texture);

    /**
     * @brief Called every frame to update the graphics.
     * @param deltaTime The time elapsed since the last update.
     */
    virtual void onGraphicsUpdate(UniformData data) override;

    /**
     * @brief Handles the shadow pass rendering for the graphics entity.
     * @param index The index of the light source for which shadows are being rendered.
     */
    virtual void onShadowPass(uint index) override;

    /**
     * @brief Handles the geometry pass rendering for the graphics entity.
     * @param data Contains uniform data needed for rendering, such as transformation
     * matrices and other parameters relevant to the geometry pass.
     */
    virtual void onGeometryPass(UniformData data) override;

    /**
     * @brief Handles the lighting pass rendering for the graphics entity
     * @param data Contains uniform data needed for lighting calculations, such as
     * light positions, colors, and any other parameters relevant to the lighting pass.
     */
    virtual void onLightingPass(UniformData data) override;

    /**
     * @brief Gets the Shininess used by this entity.
     * @return A float of the amount of shininess.
     */
    float getShininess() const;

    /**
     * @brief Sets the Shininess to be used by this entity.
     * @param shininess A float of the amount of shininess.
     */
    void setShininess(const float shininess);

protected:
    TexturePtr m_reflectiveMap; //A shared pointer to the reflective map

private:
    MeshPtr m_mesh; //A shared pointer to the mesh.
    float m_shininess = 32.0f; //Object Shininess used by the Lighting Manager and is applied to the shader
};