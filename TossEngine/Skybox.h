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
class TOSSENGINE_API Skybox : public Renderer
{
public:
    // Serialize the MeshRenderer to JSON
    json serialize() const override;

    // Deserialize the MeshRenderer from JSON
    void deserialize(const json& data) override;

    virtual void OnInspectorGUI() override
    {
        Renderer::OnInspectorGUI();

        ResourceAssignableField(m_mesh, "Mesh");
    }

    /**
     * @brief Called when the GameObject is created.
     */
    void onCreate() override;

    void Render(UniformData data, RenderingPath renderPath) override;

    void setMesh(MeshPtr mesh);

private:
    MeshPtr m_mesh;
}; 

REGISTER_COMPONENT(Skybox);