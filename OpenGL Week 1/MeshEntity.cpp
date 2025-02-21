/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2024 Media Design School
File Name : MeshEntity.cpp
Description : Entity type that renders meshes
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
**/

#include "MeshEntity.h"
#include "GraphicsEngine.h"
#include "VertexArrayObject.h" 
#include "Mesh.h"
#include "Game.h"
#include "ShadowMap.h"
#include "GeometryBuffer.h"

void MeshEntity::setMesh(const MeshPtr& mesh)
{
    m_mesh = mesh;
}

MeshPtr MeshEntity::getMesh()
{
    return m_mesh;
}

void MeshEntity::setReflectiveMapTexture(const TexturePtr& texture)
{
    m_reflectiveMap = texture;
}

void MeshEntity::onGraphicsUpdate(UniformData data)
{
    GraphicsEntity::onGraphicsUpdate(data);

    m_shader->setMat4("VPMatrix", data.projectionMatrix * data.viewMatrix);
    m_shader->setMat4("modelMatrix", m_transform.GetMatrix());

    m_shader->setVec3("CameraPos", data.cameraPosition);

    m_shader->setFloat("ObjectShininess", getShininess());
    LightManager::GetInstance().applyLighting(m_shader);


    auto& graphicsEngine = GraphicsEngine::GetInstance();
    if (m_texture != nullptr)
    {
        graphicsEngine.setTexture2D(m_texture, 0, "Texture0");
    }
    else
    {
        m_shader->setVec3("uColor", m_color);
        m_shader->setFloat("alpha", m_transparentAlpha);
    }

    auto skyboxTexture = ResourceManager::GetInstance().getSkyboxTexture();
    if (skyboxTexture)
    {
        graphicsEngine.setTextureCubeMap(skyboxTexture, 1, "Texture_Skybox");
    }

    if (m_reflectiveMap)
    {
        graphicsEngine.setTexture2D(m_reflectiveMap, 2, "ReflectionMap");
    }

    auto& lightManager = LightManager::GetInstance();
    lightManager.applyShadows(m_shader);

    auto meshVBO = m_mesh->getVertexArrayObject();
    graphicsEngine.setVertexArrayObject(meshVBO); //bind vertex buffer to graphics pipeline
    graphicsEngine.drawIndexedTriangles(TriangleType::TriangleList, meshVBO->getNumIndices());//draw triangles through the usage of index buffer

    graphicsEngine.setTexture2D(nullptr, 0, "");
    graphicsEngine.setTextureCubeMap(nullptr, 1, "");
    graphicsEngine.setTexture2D(nullptr, 2, "");
    graphicsEngine.setTexture2D(nullptr, 3, "");
}

void MeshEntity::onShadowPass(uint index)
{
    GraphicsEntity::onShadowPass(index);


    auto& lightManager = LightManager::GetInstance();
    m_shadowShader->setMat4("VPLight", lightManager.getLightSpaceMatrix(index));
    m_shadowShader->setMat4("modelMatrix", m_transform.GetMatrix());

    if (m_mesh == nullptr) return;

    // Bind the vertex array object for the mesh
    auto meshVBO = m_mesh->getVertexArrayObject();

    // Retrieve the instance of the graphics engine
    auto& graphicsEngine = GraphicsEngine::GetInstance();
    graphicsEngine.setVertexArrayObject(meshVBO);

    // Draw the mesh to update the shadow map
    graphicsEngine.drawIndexedTriangles(TriangleType::TriangleList, meshVBO->getNumIndices());
}

void MeshEntity::onGeometryPass(UniformData data)
{
    auto& graphicsEngine = GraphicsEngine::GetInstance();
    graphicsEngine.setFaceCulling(CullType::BackFace);
    graphicsEngine.setWindingOrder(WindingOrder::CounterClockWise);
    graphicsEngine.setDepthFunc(DepthType::Less);
    graphicsEngine.setShader(m_geometryShader);

    m_geometryShader->setMat4("VPMatrix", data.projectionMatrix * data.viewMatrix);
    m_geometryShader->setMat4("modelMatrix", m_transform.GetMatrix());
    m_geometryShader->setFloat("ObjectShininess", getShininess());

    if (m_texture != nullptr)
    {
        graphicsEngine.setTexture2D(m_texture, 0, "Texture0");
    }

    if (m_reflectiveMap)
    {
        graphicsEngine.setTexture2D(m_reflectiveMap, 2, "ReflectionMap");
    }

    // Bind the vertex array object for the mesh
    auto meshVBO = m_mesh->getVertexArrayObject();

    // Retrieve the instance of the graphics engine
    graphicsEngine.setVertexArrayObject(meshVBO);

    // Draw the mesh to update the shadow map
    graphicsEngine.drawIndexedTriangles(TriangleType::TriangleList, meshVBO->getNumIndices());
}

void MeshEntity::onLightingPass(UniformData data)
{
}

float MeshEntity::getShininess() const
{
    return m_shininess;
}

void MeshEntity::setShininess(const float shininess)
{
    m_shininess = shininess;
}
