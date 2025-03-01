/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2024 Media Design School
File Name : Skybox.h
Description : Skybox class represents a skybox in the 3D scene, handling its creation, updating, and shader uniform data.
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
**/

#pragma once
#include "Renderer.h"

/**
 * @class Skybox
 * @brief Represents a skybox in the 3D scene, handling its creation, updating, and shader uniform data.
 */
class Skybox : public Renderer
{
public:
    /**
     * @brief Called when the GameObject is created.
     */
    void onCreate() override;

    void Render(UniformData data, RenderingPath renderPath) override;

    void setTexture(TexturePtr texture);
    void setMesh(MeshPtr mesh);

private:
    MeshPtr m_mesh;
    TexturePtr m_texture;
};