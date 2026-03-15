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

    if (m_mesh->IsSkinned())
    {
        m_shadowShader->setBool("u_IsSkinned", true);
        UploadSkinningMatrices(m_shadowShader);
    }
    else
    {
        m_shadowShader->setBool("u_IsSkinned", false);
    }
    
    m_shadowShader->setBool("u_IsInstanced", m_mesh->IsStatic() && m_mesh->getInstanceCount() > 0);
    
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

void MeshRenderer::UploadSkinningMatrices(const ShaderPtr& shader) const
{
    if (shader == nullptr || m_mesh == nullptr || !m_mesh->IsSkinned())
    {
        return;
    }

    std::vector<Mat4> finalBoneMatrices;
    m_mesh->BuildBindPoseMatrices(finalBoneMatrices);

    for (int boneIndex = 0; boneIndex < static_cast<int>(finalBoneMatrices.size()); boneIndex++)
    {
        shader->setMat4("u_BoneMatrices[" + std::to_string(boneIndex) + "]", finalBoneMatrices[boneIndex]);
    }
}

void MeshRenderer::Render(UniformData data, RenderingPath renderPath)
{
    if (m_mesh == nullptr) return;

    auto& graphicsEngine = GraphicsEngine::GetInstance();
    graphicsEngine.setFaceCulling(CullType::BackFace); // let material able to set this
    graphicsEngine.setWindingOrder(WindingOrder::CounterClockWise);

    if (renderPath == RenderingPath::Deferred)
    {
        if (m_geometryShader == nullptr) return;

        graphicsEngine.setShader(m_geometryShader);
        m_geometryShader->setMat4("VPMatrix", data.projectionMatrix * data.viewMatrix);
        m_geometryShader->setMat4("modelMatrix", m_owner->m_transform.GetMatrix());
        
        if (m_mesh->IsSkinned())
        {
            m_geometryShader->setBool("u_IsSkinned", true);
            UploadSkinningMatrices(m_geometryShader);
        }
        else
        {
            m_geometryShader->setBool("u_IsSkinned", false);
        }
        
        m_geometryShader->setBool("u_IsInstanced", m_mesh->IsStatic() && m_mesh->getInstanceCount() > 0);
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

        if (m_mesh->IsSkinned())
        {
            shader->setBool("u_IsSkinned", true);
            UploadSkinningMatrices(shader);
        }
        else
        {
            shader->setBool("u_IsSkinned", false);
        }
        shader->setBool("u_IsInstanced", m_mesh->IsStatic() && m_mesh->getInstanceCount() > 0);
    }

    // Bind the vertex array object for the mesh
    auto meshVBO = m_mesh->getVertexArrayObject();
    if (meshVBO == nullptr) return;

    // Retrieve the instance of the graphics engine
    graphicsEngine.setVertexArrayObject(meshVBO);

    if (m_mesh->IsStatic() && m_mesh->getInstanceCount() > 0)
    {
        //Debug::Log("Instanced mesh rendering");
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
