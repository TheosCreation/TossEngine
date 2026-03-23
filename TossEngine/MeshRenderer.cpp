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

void MeshRenderer::onUpdate()
{
    if (m_mesh == nullptr || !m_mesh->IsSkinned())
    {
        return;
    }

    const std::vector<NodeTransformInfo>& nodeTransforms = m_mesh->GetNodeTransforms();
    if (nodeTransforms.empty())
    {
        return;
    }

    if (!HasValidBoneObjectsForMesh())
    {
        CreateBoneObjects();
    }
}

void MeshRenderer::onUpdateInternal()
{
    if (m_mesh == nullptr || !m_mesh->IsSkinned())
    {
        return;
    }

    const std::vector<NodeTransformInfo>& nodeTransforms = m_mesh->GetNodeTransforms();
    if (nodeTransforms.empty())
    {
        return;
    }

    if (!HasValidBoneObjectsForMesh())
    {
        CreateBoneObjects();
    }
}

bool MeshRenderer::SkeletonChanged(MeshPtr oldMesh, MeshPtr newMesh) const
{
    if (oldMesh == nullptr || newMesh == nullptr)
    {
        return true;
    }

    if (!oldMesh->IsSkinned() || !newMesh->IsSkinned())
    {
        return oldMesh->IsSkinned() != newMesh->IsSkinned();
    }

    const std::vector<NodeTransformInfo>& oldNodes = oldMesh->GetNodeTransforms();
    const std::vector<NodeTransformInfo>& newNodes = newMesh->GetNodeTransforms();

    if (oldNodes.size() != newNodes.size())
    {
        return true;
    }

    for (int nodeIndex = 0; nodeIndex < static_cast<int>(oldNodes.size()); nodeIndex++)
    {
        if (oldNodes[nodeIndex].name != newNodes[nodeIndex].name)
        {
            return true;
        }

        if (oldNodes[nodeIndex].parentIndex != newNodes[nodeIndex].parentIndex)
        {
            return true;
        }
    }

    return false;
}

void MeshRenderer::OnInspectorGUI()
{
    // Display the material from the base Renderer component.
    Renderer::OnInspectorGUI();

    ResourceAssignableField(m_mesh, "Mesh");
    if (m_mesh && m_mesh->IsSkinned())
    {
        if(ImGui::Button("Reset to default pose"))
        {
            ResetBonesToBindPose();
        }
    }
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
    if (m_mesh == nullptr || !m_mesh->IsSkinned())
    {
        return;
    }

    const std::vector<BoneInfo>& bones = m_mesh->GetBones();
    if (bones.empty())
    {
        return;
    }

    std::vector<Mat4> skinningMatrices;
    skinningMatrices.resize(bones.size(), Mat4());

    Mat4 inverseMeshWorld = m_owner->m_transform.GetMatrix().Inverse();

    for (int boneIndex = 0; boneIndex < static_cast<int>(bones.size()); boneIndex++)
    {
        const BoneInfo& boneInfo = bones[boneIndex];
        int nodeIndex = m_mesh->GetNodeIndexFromName(boneInfo.name);

        if (nodeIndex < 0 || nodeIndex >= static_cast<int>(m_boneObjects.size()))
        {
            continue;
        }

        GameObjectPtr boneObject = m_boneObjects[nodeIndex];
        if (boneObject == nullptr)
        {
            continue;
        }

        Mat4 boneWorldMatrix = boneObject->m_transform.GetMatrix();
        Mat4 boneMeshSpaceMatrix = inverseMeshWorld * boneWorldMatrix;

        skinningMatrices[boneIndex] = boneMeshSpaceMatrix * boneInfo.offsetMatrix;
    }

    shader->setMat4Array("u_BoneMatrices", skinningMatrices.data(), static_cast<int>(skinningMatrices.size()));
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
    
    if (m_mesh == mesh)
    {
        return;
    }

    bool shouldRebuildBones = false;

    if (m_mesh != nullptr && mesh != nullptr)
    {
        shouldRebuildBones = SkeletonChanged(m_mesh, mesh);
    }
    else if (m_mesh != nullptr || mesh != nullptr)
    {
        shouldRebuildBones = true;
    }

    if (shouldRebuildBones)
    {
        DestroyBoneObjects();
    }

    m_mesh = mesh;

    if (m_mesh != nullptr && m_mesh->IsSkinned())
    {
        const std::vector<NodeTransformInfo>& nodeTransforms = m_mesh->GetNodeTransforms();

        if (nodeTransforms.empty())
        {
            Debug::LogWarning(
                "MeshRenderer: skinned mesh '" + m_owner->name +
                "' has no node transforms. Bone objects will not be created."
            );
            return;
        }

        CreateBoneObjects();
    }
    else
    {
        DestroyBoneObjects();
    }
}

void MeshRenderer::CreateBoneObjects()
{
    if (m_mesh == nullptr || !m_mesh->IsSkinned())
    {
        return;
    }

    const std::vector<NodeTransformInfo>& nodeTransforms = m_mesh->GetNodeTransforms();
    if (nodeTransforms.empty())
    {
        return;
    }

    if (m_boneRootObject == nullptr)
    {
        m_boneRootObject = m_owner->getScene()->createGameObject<GameObject>(m_owner->name + "_Bones");
        m_boneRootObject->m_transform.SetParent(&m_owner->m_transform);
    }

    m_boneObjects.clear();
    m_boneObjects.resize(nodeTransforms.size(), nullptr);

    for (int nodeIndex = 0; nodeIndex < static_cast<int>(nodeTransforms.size()); nodeIndex++)
    {
        const NodeTransformInfo& nodeInfo = nodeTransforms[nodeIndex];

        GameObjectPtr boneObject = m_owner->getScene()->createGameObject<GameObject>(nodeInfo.name);
        m_boneObjects[nodeIndex] = boneObject;

        if (nodeInfo.parentIndex >= 0)
        {
            boneObject->m_transform.SetParent(&m_boneObjects[nodeInfo.parentIndex]->m_transform);
        }
        else
        {
            boneObject->m_transform.SetParent(&m_boneRootObject->m_transform);
        }

        boneObject->m_transform.SetLocalMatrix(nodeInfo.localTransform);
    }

    m_bonesCreated = true;
}

void MeshRenderer::ResetBonesToBindPose()
{
    if (m_mesh == nullptr)
    {
        return;
    }

    std::vector<NodeTransformInfo>& nodeTransforms = m_mesh->GetNodeTransforms();

    for (int nodeIndex = 0; nodeIndex < static_cast<int>(nodeTransforms.size()); nodeIndex++)
    {
        if (nodeIndex < 0 || nodeIndex >= static_cast<int>(m_boneObjects.size()))
        {
            continue;
        }

        GameObjectPtr boneObject = m_boneObjects[nodeIndex];
        if (boneObject == nullptr)
        {
            continue;
        }

        boneObject->m_transform.SetLocalMatrix(nodeTransforms[nodeIndex].localTransform);
    }
}

bool MeshRenderer::HasValidBoneObjectsForMesh() const
{
    if (m_mesh == nullptr || !m_mesh->IsSkinned())
    {
        return false;
    }

    const std::vector<NodeTransformInfo>& nodeTransforms = m_mesh->GetNodeTransforms();

    if (nodeTransforms.empty())
    {
        return false;
    }

    if (m_boneObjects.size() != nodeTransforms.size())
    {
        return false;
    }

    if (m_boneRootObject == nullptr)
    {
        return false;
    }

    for (int nodeIndex = 0; nodeIndex < static_cast<int>(nodeTransforms.size()); nodeIndex++)
    {
        const NodeTransformInfo& nodeInfo = nodeTransforms[nodeIndex];
        GameObjectPtr boneObject = m_boneObjects[nodeIndex];

        if (boneObject == nullptr)
        {
            return false;
        }

        if (boneObject->name != nodeInfo.name)
        {
            return false;
        }
    }

    return true;
}

void MeshRenderer::DestroyBoneObjects()
{
    for (int boneIndex = 0; boneIndex < static_cast<int>(m_boneObjects.size()); boneIndex++)
    {
        GameObjectPtr boneObject = m_boneObjects[boneIndex];
        if (boneObject != nullptr)
        {
            m_owner->getScene()->deleteGameObject(boneObject);
        }
    }

    m_boneObjects.clear();

    if (m_boneRootObject != nullptr)
    {
        m_owner->getScene()->deleteGameObject(m_boneRootObject);
        m_boneRootObject = nullptr;
    }

    m_bonesCreated = false;
}

MeshPtr MeshRenderer::GetMesh() const
{
    return m_mesh;
}

GameObjectPtr MeshRenderer::GetBoneObjectByName(const std::string& boneName) const
{
    if (m_mesh == nullptr)
    {
        return nullptr;
    }

    int nodeIndex = m_mesh->GetNodeIndexFromName(boneName);
    if (nodeIndex < 0 || nodeIndex >= static_cast<int>(m_boneObjects.size()))
    {
        return nullptr;
    }

    return m_boneObjects[nodeIndex];
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
