/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2024 Media Design School
File Name : InstancedMeshEntity.cpp
Description : Entity type that renders instanced meshes
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
**/

#include "InstancedMeshEntity.h"
#include "GraphicsEngine.h"
#include "InstancedMesh.h"
#include "VertexArrayObject.h" 
#include "Game.h"

void InstancedMeshEntity::setMesh(const InstancedMeshPtr& mesh)
{
    m_mesh = mesh;
}

InstancedMeshPtr InstancedMeshEntity::getMesh()
{
	return m_mesh;
}

void InstancedMeshEntity::onCreate()
{
}

void InstancedMeshEntity::setUniformData(UniformData data)
{
}

void InstancedMeshEntity::onGraphicsUpdate(UniformData data)
{
    GraphicsEntity::onGraphicsUpdate(data);

    m_shader->setMat4("VPMatrix", data.projectionMatrix * data.viewMatrix);
    m_shader->setVec3("CameraPos", data.cameraPosition);
    m_shader->setFloat("ObjectShininess", getShininess());

    LightManager::GetInstance().applyLighting(m_shader);

    auto& graphicsEngine = GraphicsEngine::GetInstance();
    if (m_texture != nullptr)
    {
        graphicsEngine.setTexture2D(m_texture, 0, "Texture0");
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
    graphicsEngine.drawIndexedTrianglesInstanced(TriangleType::TriangleList, meshVBO->getNumIndices(), m_mesh->getInstanceCount());


    graphicsEngine.setTexture2D(nullptr, 0, "");
    graphicsEngine.setTextureCubeMap(nullptr, 1, "");
    graphicsEngine.setTexture2D(nullptr, 2, "");
    graphicsEngine.setTexture2D(nullptr, 3, "");
}

void InstancedMeshEntity::onGeometryPass(UniformData data)
{
    auto& graphicsEngine = GraphicsEngine::GetInstance();
    graphicsEngine.setFaceCulling(CullType::BackFace);
    graphicsEngine.setWindingOrder(WindingOrder::CounterClockWise);
    graphicsEngine.setDepthFunc(DepthType::Less);
    graphicsEngine.setShader(m_geometryShader);

    m_geometryShader->setMat4("VPMatrix", data.projectionMatrix * data.viewMatrix);
    m_geometryShader->setFloat("ObjectShininess", getShininess());

    if (m_texture != nullptr)
    {
        graphicsEngine.setTexture2D(m_texture, 0, "Texture0");
    }

    auto meshVBO = m_mesh->getVertexArrayObject();
    graphicsEngine.setVertexArrayObject(meshVBO); //bind vertex buffer to graphics pipeline
    graphicsEngine.drawIndexedTrianglesInstanced(TriangleType::TriangleList, meshVBO->getNumIndices(), m_mesh->getInstanceCount());

    graphicsEngine.setTexture2D(nullptr, 0, "");
}

void InstancedMeshEntity::onShadowPass(uint index)
{
    GraphicsEntity::onShadowPass(index);

    // Retrieve the instance of the graphics engine
    auto& graphicsEngine = GraphicsEngine::GetInstance();

    auto& lightManager = LightManager::GetInstance();
    m_shadowShader->setMat4("VPLight", lightManager.getLightSpaceMatrix(index));

    if (m_mesh == nullptr) return;

    auto meshVBO = m_mesh->getVertexArrayObject();
    graphicsEngine.setVertexArrayObject(meshVBO); //bind vertex buffer to graphics pipeline
    graphicsEngine.drawIndexedTrianglesInstanced(TriangleType::TriangleList, meshVBO->getNumIndices(), m_mesh->getInstanceCount());
}