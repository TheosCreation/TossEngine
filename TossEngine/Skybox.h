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
#include "Mesh.h"

/**
 * @class Skybox
 * @brief Represents a skybox in the 3D scene, handling its creation, updating, and shader uniform data.
 */
class TOSSENGINE_API Skybox : public Renderer
{
public:
    virtual void OnInspectorGUI() override
    {
        Renderer::OnInspectorGUI();

        ResourceAssignableField(m_mesh, "Mesh");
        ResourceAssignableField(m_texture, "Cubemap");
    }

    /**
     * @brief Called when the GameObject is created.
     */
    void onCreate() override;

    void Render(UniformData data, RenderingPath renderPath) override;

    void setMesh(const MeshPtr& mesh);
    void setTextureCubeMap(const shared_ptr<TextureCubeMap>& texture);

private:
    MeshPtr m_mesh;
	TextureCubeMapPtr m_texture;

    SERIALIZABLE_MEMBERS(m_mesh, m_material, m_texture)
}; 

REGISTER_COMPONENT(Skybox);