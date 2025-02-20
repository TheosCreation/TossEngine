/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2024 Media Design School
File Name : SkyboxEntity.h
Description : SkyboxEntity class represents a skybox in the 3D scene, handling its creation, updating, and shader uniform data.
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
**/

#pragma once
#include "MeshEntity.h"

/**
 * @class SkyboxEntity
 * @brief Represents a skybox in the 3D scene, handling its creation, updating, and shader uniform data.
 */
class SkyboxEntity : public MeshEntity
{
public:
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
    * @brief Called during the geometry pass.
    * @param data The uniform data for the pass.
    */
    void onGeometryPass(UniformData data) override;
};