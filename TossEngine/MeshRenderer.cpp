#include "MeshRenderer.h"

#include <algorithm>
#include "ResourceManager.h"
#include "GraphicsEngine.h"
#include "LightManager.h"
#include "Mesh.h"
#include "Shader.h"
#include "GameObject.h"
#include "Scene.h"
#include "VertexArrayObject.h"

void MeshRenderer::onCreateLate()
{
    auto& resourceManager = ResourceManager::GetInstance();
    m_geometryShader = resourceManager.get<Shader>("GeometryPassMeshShader");
    m_shadowShader = resourceManager.get<Shader>("ShadowShader");
}

void MeshRenderer::onShadowPass(uint index)
{
    if (m_shadowShader == nullptr || m_mesh == nullptr) return;

    auto& graphicsEngine = GraphicsEngine::GetInstance();
    graphicsEngine.setFaceCulling(CullType::BackFace);
    graphicsEngine.setWindingOrder(WindingOrder::CounterClockWise);
    graphicsEngine.setDepthFunc(DepthType::Less);
    graphicsEngine.setShader(m_shadowShader);

    m_shadowShader->setMat4("VPLight", m_owner->getLightManager()->getLightSpaceMatrix(index));
    m_shadowShader->setMat4("modelMatrix", m_owner->m_transform.GetMatrix());

    // Bind the vertex array object for the mesh
    auto meshVBO = m_mesh->getVertexArrayObject();
    if (meshVBO == nullptr) return;

    graphicsEngine.setVertexArrayObject(meshVBO);

    // Draw the mesh to update the shadow map
    if (m_mesh->getInstanceCount() > 0)
    {
        // mesh has instances so we render instanced
        graphicsEngine.drawIndexedTrianglesInstanced(TriangleType::TriangleList, meshVBO->getNumIndices(), m_mesh->getInstanceCount());
    }
    else
    {
        graphicsEngine.drawIndexedTriangles(TriangleType::TriangleList, meshVBO->getNumIndices());
    }
}

void MeshRenderer::Render(UniformData data, RenderingPath renderPath)
{
    if (m_mesh == nullptr) return;

    auto& graphicsEngine = GraphicsEngine::GetInstance();
    graphicsEngine.setFaceCulling(CullType::BackFace);
    graphicsEngine.setWindingOrder(WindingOrder::CounterClockWise);

    if (renderPath == RenderingPath::Deferred)
    {
        if (m_geometryShader == nullptr) return;

        graphicsEngine.setShader(m_geometryShader);
        m_geometryShader->setMat4("VPMatrix", data.projectionMatrix * data.viewMatrix);
        m_geometryShader->setMat4("modelMatrix", m_owner->m_transform.GetMatrix());
    }


    // TODO: migrate to use material fully
    if (renderPath == RenderingPath::Forward)
    {
        if (m_material == nullptr) return;
        
        if (!m_material->Bind())
        {
            return;
        }
        
        auto shader = m_material->GetShader();
        shader->setMat4("VPMatrix", data.projectionMatrix * data.viewMatrix);
        shader->setMat4("modelMatrix", m_owner->m_transform.GetMatrix());
        shader->setVec3("CameraPos", data.cameraPosition);

        m_owner->getLightManager()->applyLighting(shader);

        auto skyboxTexture = m_owner->getScene()->getSkyBoxTexture();
        if (skyboxTexture)
        {
            graphicsEngine.setTextureCubeMap(skyboxTexture, 6, "Texture_Skybox");
        }

        m_owner->getLightManager()->applyShadows(shader);
    }

    // Bind the vertex array object for the mesh
    auto meshVBO = m_mesh->getVertexArrayObject();
    if (meshVBO == nullptr) return;

    // Retrieve the instance of the graphics engine
    graphicsEngine.setVertexArrayObject(meshVBO);

    if (m_mesh->getInstanceCount() > 0)
    {
        // mesh has instances so we render instanced
        graphicsEngine.drawIndexedTrianglesInstanced(TriangleType::TriangleList, meshVBO->getNumIndices(), m_mesh->getInstanceCount());
    }
    else
    {
        graphicsEngine.drawIndexedTriangles(TriangleType::TriangleList, meshVBO->getNumIndices());
    }

    graphicsEngine.setTextureCubeMap(nullptr, 6, "Texture_Skybox");
    graphicsEngine.setTexture2D(nullptr, 1, "ReflectionMap");
}

void MeshRenderer::SetMesh(MeshPtr mesh)
{
    m_mesh = mesh;
}

MeshPtr MeshRenderer::GetMesh() const
{
    return m_mesh;
}

float MeshRenderer::GetAlpha() const
{
    if (m_material != nullptr) return m_material->GetFloatBinding("Alpha");
    return 1.0f;
}

Vector3 MeshRenderer::GetExtent()
{
    if (!m_mesh)                 return Vector3(0.5f);
    return m_mesh->GetExtent();
}
