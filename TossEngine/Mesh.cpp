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

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

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

    std::vector<VertexMesh> list_vertices;
    std::vector<uint> list_indices;

    Vector3 minBounds(FLT_MAX, FLT_MAX, FLT_MAX);
    Vector3 maxBounds(-FLT_MAX, -FLT_MAX, -FLT_MAX);

    aiMatrix4x4 identityTransform;
    ProcessAssimpNode(
        scene->mRootNode,
        scene,
        identityTransform,
        list_vertices,
        list_indices,
        minBounds,
        maxBounds,
        m_scale
    );

    if (list_vertices.empty() || list_indices.empty())
    {
        Debug::LogError("Mesh | No valid geometry found in: " + m_path, false);
        isLoaded = false;
        return;
    }

    const VertexAttribute attribsList[] = {
        { 3 },
        { 2 },
        { 3 }
    };

    m_vao = GraphicsEngine::GetInstance().createVertexArrayObject(
        {
            static_cast<void*>(list_vertices.data()),
            sizeof(VertexMesh),
            static_cast<uint>(list_vertices.size()),
            const_cast<VertexAttribute*>(attribsList),
            3
        },
        {
            static_cast<void*>(list_indices.data()),
            static_cast<uint>(list_indices.size())
        }
    );

    extent = Vector3(
        (maxBounds.x - minBounds.x) * 0.5f,
        (maxBounds.y - minBounds.y) * 0.5f,
        (maxBounds.z - minBounds.z) * 0.5f
    );

    initInstanceBuffer();
    m_scaleTemp = m_scale;
    isLoaded = true;
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
            auto chosen = TossEngine::GetInstance().openFileDialog("*.obj");
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


    if (ImGui::CollapsingHeader("Mesh Instances", ImGuiTreeNodeFlags_DefaultOpen))
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
