/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2024 Media Design School
File Name : InstancedMeshEntity.h
Description : Entity type that renders instanced meshes
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
**/

#pragma once

#include "MeshEntity.h"

/**
 * @class InstancedMeshEntity
 * @brief An entity that renders instanced meshes.
 */
class InstancedMeshEntity : public MeshEntity
{
public:
    /**
     * @brief Sets the instanced mesh for this entity.
     * @param mesh A shared pointer to the instanced mesh.
     */
    void setMesh(const InstancedMeshPtr& mesh);

    /**
     * @brief Gets the instanced mesh for this entity.
     * @return A shared pointer to the instanced mesh.
     */
    InstancedMeshPtr getMesh();

    /**
     * @brief Called when the entity is created.
     */
    void onCreate() override;

    /**
     * @brief Sets the uniform data for the shader.
     * @param data The uniform data to set.
     */
    void setUniformData(UniformData data) override;


    /**
     * @brief Called every frame to update the graphics.
     * @param deltaTime The time elapsed since the last update.
     */
    void onGraphicsUpdate(UniformData data) override;
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

private:
    InstancedMeshPtr m_mesh; //A shared pointer to the instanced mesh.
};