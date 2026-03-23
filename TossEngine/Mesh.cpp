/***
DeviousDevs
Auckland
New Zealand
(c) 2026 DeviousDevs
File Name : Mesh.cpp
Description : A class representing a mesh resource
Author : Theo Morris
Mail : theo.morris@outlook.co.nz
**/

#include "Mesh.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <filesystem>
#include "VertexMesh.h"
#include "GraphicsEngine.h"
#include "TossEngine.h"

static void ProcessAssimpMesh(
    aiMesh* mesh,
    const aiMatrix4x4& worldTransform,
    std::vector<VertexMesh>& outVertices,
    std::vector<uint>& outIndices,
    Vector3& minBounds,
    Vector3& maxBounds,
    float scale)
{
    
    uint baseVertex = static_cast<uint>(outVertices.size());

    aiMatrix3x3 normalMatrix = aiMatrix3x3(worldTransform);
    normalMatrix.Inverse();
    normalMatrix.Transpose();

    for (uint vertexIndex = 0; vertexIndex < mesh->mNumVertices; vertexIndex++)
    {
        aiVector3D position = mesh->mVertices[vertexIndex];
        position *= scale;
        position = worldTransform * position;

        Vector2 texCoord(0.0f, 0.0f);
        if (mesh->HasTextureCoords(0))
        {
            texCoord.x = mesh->mTextureCoords[0][vertexIndex].x;
            texCoord.y = mesh->mTextureCoords[0][vertexIndex].y;
        }

        Vector3 normal(0.0f);
        if (mesh->HasNormals())
        {
            aiVector3D assimpNormal = normalMatrix * mesh->mNormals[vertexIndex];
            assimpNormal.Normalize();

            normal.x = assimpNormal.x;
            normal.y = assimpNormal.y;
            normal.z = assimpNormal.z;
        }

        Vector3 finalPosition(position.x, position.y, position.z);
        VertexMesh vertex(finalPosition, texCoord, normal);
        outVertices.push_back(vertex);

        minBounds.x = std::min(minBounds.x, finalPosition.x);
        minBounds.y = std::min(minBounds.y, finalPosition.y);
        minBounds.z = std::min(minBounds.z, finalPosition.z);

        maxBounds.x = std::max(maxBounds.x, finalPosition.x);
        maxBounds.y = std::max(maxBounds.y, finalPosition.y);
        maxBounds.z = std::max(maxBounds.z, finalPosition.z);
    }

    for (uint faceIndex = 0; faceIndex < mesh->mNumFaces; faceIndex++)
    {
        const aiFace& face = mesh->mFaces[faceIndex];

        if (face.mNumIndices != 3)
        {
            continue;
        }

        outIndices.push_back(baseVertex + face.mIndices[0]);
        outIndices.push_back(baseVertex + face.mIndices[1]);
        outIndices.push_back(baseVertex + face.mIndices[2]);
    }
}

static void ProcessAssimpNode(
    aiNode* node,
    const aiScene* scene,
    const aiMatrix4x4& parentTransform,
    std::vector<VertexMesh>& outVertices,
    std::vector<uint>& outIndices,
    Vector3& minBounds,
    Vector3& maxBounds,
    float scale)
{
    aiMatrix4x4 worldTransform = parentTransform * node->mTransformation;

    for (uint meshIndex = 0; meshIndex < node->mNumMeshes; meshIndex++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[meshIndex]];
        if (mesh != nullptr)
        {
            ProcessAssimpMesh(
                mesh,
                worldTransform,
                outVertices,
                outIndices,
                minBounds,
                maxBounds,
                scale
            );
        }
    }

    for (uint childIndex = 0; childIndex < node->mNumChildren; childIndex++)
    {
        ProcessAssimpNode(
            node->mChildren[childIndex],
            scene,
            worldTransform,
            outVertices,
            outIndices,
            minBounds,
            maxBounds,
            scale
        );
    }
}

static void BuildNodeTransformMap(
    aiNode* node,
    int parentIndex,
    const Mat4& parentGlobalTransform,
    std::vector<NodeTransformInfo>& outNodes,
    std::unordered_map<std::string, int>& outNodeNameToIndex)
{
    NodeTransformInfo info;
    info.name = node->mName.C_Str();
    info.parentIndex = parentIndex;
    info.localTransform = Mat4(node->mTransformation);
    info.globalTransform = parentGlobalTransform * info.localTransform;

    int thisIndex = static_cast<int>(outNodes.size());
    outNodes.push_back(info);
    outNodeNameToIndex[info.name] = thisIndex;

    for (uint childIndex = 0; childIndex < node->mNumChildren; childIndex++)
    {
        BuildNodeTransformMap(
            node->mChildren[childIndex],
            thisIndex,
            info.globalTransform,
            outNodes,
            outNodeNameToIndex);
    }
}

static void AddBoneInfluence(VertexSkinned& vertex, int boneIndex, float boneWeight)
{
    if (vertex.boneWeights.x == 0.0f)
    {
        vertex.boneIndices.x = boneIndex;
        vertex.boneWeights.x = boneWeight;
        return;
    }

    if (vertex.boneWeights.y == 0.0f)
    {
        vertex.boneIndices.y = boneIndex;
        vertex.boneWeights.y = boneWeight;
        return;
    }

    if (vertex.boneWeights.z == 0.0f)
    {
        vertex.boneIndices.z = boneIndex;
        vertex.boneWeights.z = boneWeight;
        return;
    }

    if (vertex.boneWeights.w == 0.0f)
    {
        vertex.boneIndices.w = boneIndex;
        vertex.boneWeights.w = boneWeight;
        return;
    }
}

static void NormalizeWeights(VertexSkinned& vertex)
{
    float totalWeight = vertex.boneWeights.x + vertex.boneWeights.y + vertex.boneWeights.z + vertex.boneWeights.w;
    if (totalWeight <= 0.0f)
    {
        return;
    }

    float invTotalWeight = 1.0f / totalWeight;
    vertex.boneWeights.x *= invTotalWeight;
    vertex.boneWeights.y *= invTotalWeight;
    vertex.boneWeights.z *= invTotalWeight;
    vertex.boneWeights.w *= invTotalWeight;
}

static void BuildSkeletonRecursive(aiNode* node, int parentIndex, Skeleton& skeleton)
{
    SkeletonBone bone;
    bone.name = node->mName.C_Str();
    bone.parentIndex = parentIndex;
    bone.localBindTransform = Mat4(node->mTransformation);

    int thisIndex = static_cast<int>(skeleton.bones.size());
    skeleton.bones.push_back(bone);

    for (uint childIndex = 0; childIndex < node->mNumChildren; childIndex++)
    {
        BuildSkeletonRecursive(node->mChildren[childIndex], thisIndex, skeleton);
    }
}

Mesh::Mesh(const std::string& uid, ResourceManager* mgr) : Resource(uid, mgr)
{
}

void Mesh::onCreateLate()
{
    if (m_path.empty()) return;
    LoadMeshFromFilePath();
}

void Mesh::LoadMeshFromFilePath()
{
    if (m_path.empty())
    {
        return;
    }

    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(
        m_path,
        aiProcess_Triangulate |
        aiProcess_GenSmoothNormals |
        aiProcess_JoinIdenticalVertices |
        aiProcess_ImproveCacheLocality |
        aiProcess_CalcTangentSpace |
        aiProcess_FlipUVs |
        aiProcess_SortByPType
    );

    if (scene == nullptr || scene->mRootNode == nullptr)
    {
        Debug::LogError("Mesh | Failed to load: " + m_path + " | " + importer.GetErrorString(), false);
        isLoaded = false;
        return;
    }

    m_vao = nullptr;
    m_bones.clear();
    m_animationClips.clear();
    m_skeleton.bones.clear();
    extent = Vector3(0.0f);

    bool containsBones = false;

    for (uint meshIndex = 0; meshIndex < scene->mNumMeshes; meshIndex++)
    {
        aiMesh* assimpMesh = scene->mMeshes[meshIndex];
        if (assimpMesh != nullptr && assimpMesh->HasBones())
        {
            containsBones = true;
            break;
        }
    }

    if (containsBones)
    {
        m_meshType = MeshType::Skinned;
        LoadSkinnedMesh(scene);
    }
    else
    {
        m_meshType = MeshType::Static;
        LoadStaticMesh(scene);
    }

    m_scaleTemp = m_scale;
}

Mesh::~Mesh()
{
}

void Mesh::OnInspectorGUI()
{
    ImGui::Text(("Mesh Inspector - ID: " + m_uniqueID).c_str());
    ImGui::Separator();

    if (ImGui::BeginTable("MeshFilepath", 3,
        ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg))
    {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Filepath:");
        ImGui::TableSetColumnIndex(1);
        if (m_path.empty()) {
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "Not Assigned");
        }
        else {
            ImGui::TextUnformatted(m_path.c_str());
        }
        ImGui::TableSetColumnIndex(2);
        if (ImGui::Button("Browse##MeshFilePath")) {
            auto chosen = TossEngine::GetInstance().openFileDialog("*.obj;*.fbx;*.gltf;*.glb");
            if (!chosen.empty()) {
                auto root = getProjectRoot();
                auto relPath = std::filesystem::relative(chosen, root);
                m_path = relPath.string();
                if (!m_path.empty()) LoadMeshFromFilePath();
            }
        }

        ImGui::EndTable();
    }

    if (ImGui::SliderFloat("Import Scale", &m_scaleTemp, 0.01f, 10.0f, "%.3f")) {
        // just tracking edits; no apply yet
    }
    const bool sliderActive = ImGui::IsItemActive();       // is user still dragging?
    const bool hasChange = (m_scaleTemp != m_scale);    // dirty?

    if (hasChange) {
        ImGui::SameLine();
        if (sliderActive) ImGui::BeginDisabled();          // "not able to be pressed" while dragging
        if (ImGui::Button("Apply")) {
            m_scale = m_scaleTemp;                         // commit
            if (!m_path.empty()) LoadMeshFromFilePath();   // rebuild with new scale
        }
        if (sliderActive) ImGui::EndDisabled();
    }
    
    if (IsSkinned())
    {
        ImGui::Separator();
        ImGui::TextUnformatted("Mesh Type: Skinned");
        ImGui::Text("Bone Count: %d", static_cast<int>(m_bones.size()));
        ImGui::Text("Animation Clip Count: %d", static_cast<int>(m_animationClips.size()));
    }
    else
    {
        ImGui::Separator();
        ImGui::TextUnformatted("Mesh Type: Static");
    }

    if (IsStatic() && ImGui::CollapsingHeader("Mesh Instances", ImGuiTreeNodeFlags_DefaultOpen))
    {
        // Display instance count
        ImGui::Text("Instance Count: %d", getInstanceCount());

        // Button to add an instance with default transform
        if (ImGui::Button("Add Instance"))
        {
            addInstance(Vector3(0, 0, 0), Vector3(1, 1, 1), Vector3(0, 0, 0));
            initInstanceBuffer();
        }

        ImGui::SameLine();

        // Button to clear all instances
        if (ImGui::Button("Clear Instances"))
        {
            clearInstances();
            initInstanceBuffer();
        }

        bool m_instanceBufferDirty = false;

        ImGui::Separator();
        if (ImGui::TreeNode("Instance Transforms"))
        {
            int index = 0;
            for (Transform& transform : m_instanceTransforms)
            {
                if (ImGui::TreeNode(("Instance " + std::to_string(index)).c_str()))
                {
                    if (ImGui::DragFloat3("Position", transform.localPosition.Data(), 0.1f))
                    {
                        m_instanceBufferDirty = true;
                    }
                    
                    if (ImGui::DragFloat3("Rotation", eulerAngles.Data(), 0.1f))
                    {
                        transform.localRotation = Quaternion(eulerAngles.ToRadians());
                        m_instanceBufferDirty = true;
                    }
                    else
                    {
                        eulerAngles = transform.localRotation.Normalized().ToEulerAngles().ToDegrees();
                    }

                    if (ImGui::DragFloat3("Scale", transform.localScale.Data(), 0.1f))
                    {
                        m_instanceBufferDirty = true;
                    }

                    ImGui::TreePop();
                }
                index++;
            }
            ImGui::TreePop();
        }

        if (m_instanceBufferDirty)
        {
            initInstanceBuffer();
            m_instanceBufferDirty = false;
        }
    }
}

bool Mesh::Delete(bool deleteSelf)
{
    return false;
}

VertexArrayObjectPtr Mesh::getVertexArrayObject()
{
	return m_vao;
}

void Mesh::addInstance(Vector3 position, Vector3 scale, Vector3 rotation)
{
    Transform transform;
    transform.localPosition = position;
    transform.localScale = scale;
    transform.localRotation = Quaternion(rotation);
    m_instanceTransforms.push_back(transform);
}

void Mesh::initInstanceBuffer() const
{
    if (m_instanceTransforms.empty()) return;

    std::vector<Mat4> matrices;
    matrices.reserve(m_instanceTransforms.size());

    for (const auto& transform : m_instanceTransforms)
    {
        matrices.push_back(transform.GetMatrix());
    }

    m_vao->initInstanceBuffer(matrices.data(), matrices.size());
}

int Mesh::getInstanceCount() const
{
    return static_cast<int>(m_instanceTransforms.size());
}

std::vector<Transform> Mesh::getInstanceTransforms() const {
    return m_instanceTransforms;
}

void Mesh::clearInstances()
{
    m_instanceTransforms.clear();
}

Vector3 Mesh::GetExtent()
{
    return extent;
}

bool Mesh::IsSkinned() const
{
    return m_meshType == MeshType::Skinned;
}

bool Mesh::IsStatic() const
{
    return m_meshType == MeshType::Static;
}

const Skeleton& Mesh::GetSkeleton() const
{
    return m_skeleton;
}

const std::vector<BoneInfo>& Mesh::GetBones() const
{
    return m_bones;
}

void Mesh::BuildBindPoseMatrices(std::vector<Mat4>& outFinalBoneMatrices) const
{
    
    outFinalBoneMatrices.clear();

    if (!IsSkinned())
    {
        return;
    }

    outFinalBoneMatrices.resize(m_bones.size(), Mat4());

    for (const BoneInfo& boneInfo : m_bones)
    {
        auto nodeIterator = m_nodeNameToIndex.find(boneInfo.name);
        if (nodeIterator == m_nodeNameToIndex.end())
        {
            continue;
        }

        const NodeTransformInfo& nodeInfo = m_nodeTransforms[nodeIterator->second];

        outFinalBoneMatrices[boneInfo.index] =
            nodeInfo.globalTransform *
            boneInfo.inverseBindMatrix;
    }
}

std::vector<NodeTransformInfo>& Mesh::GetNodeTransforms()
{
    return m_nodeTransforms;
}

void Mesh::LoadStaticMesh(const aiScene* scene)
{
    std::vector<VertexMesh> vertices;
    std::vector<uint> indices;

    Vector3 minBounds(FLT_MAX, FLT_MAX, FLT_MAX);
    Vector3 maxBounds(-FLT_MAX, -FLT_MAX, -FLT_MAX);

    aiMatrix4x4 identityTransform;
    ProcessAssimpNode(
        scene->mRootNode,
        scene,
        identityTransform,
        vertices,
        indices,
        minBounds,
        maxBounds,
        m_scale
    );

    if (vertices.empty() || indices.empty())
    {
        Debug::LogError("Mesh | No valid static geometry found in: " + m_path, false);
        isLoaded = false;
        return;
    }

    const VertexAttribute attribsList[] =
    {
        { 3 },
        { 2 },
        { 3 }
    };

    m_vao = GraphicsEngine::GetInstance().createVertexArrayObject(
        {
            static_cast<void*>(vertices.data()),
            sizeof(VertexMesh),
            static_cast<uint>(vertices.size()),
            const_cast<VertexAttribute*>(attribsList),
            3
        },
        {
            static_cast<void*>(indices.data()),
            static_cast<uint>(indices.size())
        }
    );

    extent = Vector3(
        (maxBounds.x - minBounds.x) * 0.5f,
        (maxBounds.y - minBounds.y) * 0.5f,
        (maxBounds.z - minBounds.z) * 0.5f
    );

    initInstanceBuffer();
    isLoaded = true;
}

void Mesh::LoadSkinnedMesh(const aiScene* scene)
{
    std::vector<VertexSkinned> vertices;
    std::vector<uint> indices;

    Vector3 minBounds(FLT_MAX, FLT_MAX, FLT_MAX);
    Vector3 maxBounds(-FLT_MAX, -FLT_MAX, -FLT_MAX);

    m_bones.clear();
    m_skeleton.Clear();

    BuildSkeletonRecursive(scene->mRootNode, -1, m_skeleton);

    std::unordered_map<std::string, int> boneNameToIndex;

    for (uint meshIndex = 0; meshIndex < scene->mNumMeshes; meshIndex++)
    {
        aiMesh* assimpMesh = scene->mMeshes[meshIndex];
        if (assimpMesh == nullptr)
        {
            continue;
        }

        uint baseVertex = static_cast<uint>(vertices.size());

        for (uint vertexIndex = 0; vertexIndex < assimpMesh->mNumVertices; vertexIndex++)
        {
            aiVector3D position = assimpMesh->mVertices[vertexIndex];

            Vector2 texCoord(0.0f, 0.0f);
            if (assimpMesh->HasTextureCoords(0))
            {
                texCoord.x = assimpMesh->mTextureCoords[0][vertexIndex].x;
                texCoord.y = assimpMesh->mTextureCoords[0][vertexIndex].y;
            }

            Vector3 normal(0.0f);
            if (assimpMesh->HasNormals())
            {
                normal.x = assimpMesh->mNormals[vertexIndex].x;
                normal.y = assimpMesh->mNormals[vertexIndex].y;
                normal.z = assimpMesh->mNormals[vertexIndex].z;
            }

            Vector3 finalPosition(position.x, position.y, position.z);
            VertexSkinned vertex(finalPosition, texCoord, normal);
            vertices.push_back(vertex);

            minBounds.x = std::min(minBounds.x, finalPosition.x);
            minBounds.y = std::min(minBounds.y, finalPosition.y);
            minBounds.z = std::min(minBounds.z, finalPosition.z);

            maxBounds.x = std::max(maxBounds.x, finalPosition.x);
            maxBounds.y = std::max(maxBounds.y, finalPosition.y);
            maxBounds.z = std::max(maxBounds.z, finalPosition.z);
        }

        for (uint boneIndex = 0; boneIndex < assimpMesh->mNumBones; boneIndex++)
        {
            aiBone* assimpBone = assimpMesh->mBones[boneIndex];
            if (assimpBone == nullptr)
            {
                continue;
            }

            std::string boneName = assimpBone->mName.C_Str();

            int finalBoneIndex = -1;

            auto boneIterator = boneNameToIndex.find(boneName);
            if (boneIterator == boneNameToIndex.end())
            {
                BoneInfo boneInfo;
                boneInfo.name = boneName;
                boneInfo.index = static_cast<int>(m_bones.size());
                boneInfo.inverseBindMatrix = Mat4(assimpBone->mOffsetMatrix);

                finalBoneIndex = boneInfo.index;
                m_bones.push_back(boneInfo);
                boneNameToIndex.emplace(boneName, finalBoneIndex);
            }
            else
            {
                finalBoneIndex = boneIterator->second;
            }

            for (uint weightIndex = 0; weightIndex < assimpBone->mNumWeights; weightIndex++)
            {
                const aiVertexWeight& weight = assimpBone->mWeights[weightIndex];
                uint finalVertexIndex = baseVertex + weight.mVertexId;
                AddBoneInfluence(vertices[finalVertexIndex], finalBoneIndex, weight.mWeight);
            }
        }

        for (uint faceIndex = 0; faceIndex < assimpMesh->mNumFaces; faceIndex++)
        {
            const aiFace& face = assimpMesh->mFaces[faceIndex];
            if (face.mNumIndices != 3)
            {
                continue;
            }

            indices.push_back(baseVertex + face.mIndices[0]);
            indices.push_back(baseVertex + face.mIndices[1]);
            indices.push_back(baseVertex + face.mIndices[2]);
        }
    }

    for (VertexSkinned& vertex : vertices)
    {
        NormalizeWeights(vertex);
    }

    if (vertices.empty() || indices.empty())
    {
        Debug::LogError("Mesh | No valid skinned geometry found in: " + m_path, false);
        isLoaded = false;
        return;
    }

    const VertexAttribute attribsList[] =
    {
        { 3 },
        { 2 },
        { 3 },
        { 4, VertexAttributeType::Int },
        { 4 }
    };

    m_vao = GraphicsEngine::GetInstance().createVertexArrayObject(
        {
            static_cast<void*>(vertices.data()),
            sizeof(VertexSkinned),
            static_cast<uint>(vertices.size()),
            const_cast<VertexAttribute*>(attribsList),
            5
        },
        {
            static_cast<void*>(indices.data()),
            static_cast<uint>(indices.size())
        }
    );

    extent = Vector3(
        (maxBounds.x - minBounds.x) * 0.5f,
        (maxBounds.y - minBounds.y) * 0.5f,
        (maxBounds.z - minBounds.z) * 0.5f
    );

    isLoaded = true;
}