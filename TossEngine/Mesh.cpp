/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2024 Media Design School
File Name : Mesh.cpp
Description : A class representing a mesh resource
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
**/

#include "Mesh.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <filesystem>
#include "VertexMesh.h"
#include "GraphicsEngine.h"

Mesh::Mesh(const MeshDesc& desc, const string& uniqueId, ResourceManager* manager) : Resource(desc.filePath, uniqueId, manager)
{
    tinyobj::attrib_t attribs;
    std::vector<tinyobj::shape_t> shapes;
    
    std::string warn;
    std::string err;
    
    auto inputfile = std::filesystem::path(desc.filePath).string();
    
    bool res = tinyobj::LoadObj(&attribs, &shapes, nullptr, &warn, &err, inputfile.c_str(), nullptr);

    // Log any warnings if they exist
    if (!warn.empty()) {
        Debug::LogWarning("Mesh | Warning(s): " + warn);
    }

    // Log any errors if they exist
    if (!err.empty()) {
        Debug::LogError("Mesh | Creation failed with the following error(s): " + err, false);
    }

    // Check if the result was unsuccessful and log an additional message if needed
    if (!res) {
        Debug::LogError("Mesh | not created successfully.", false);
    }
    
    std::vector<VertexMesh> list_vertices;
    std::vector<uint> list_indices;
    
    size_t vertex_buffer_size = 0;
    
    for (size_t s = 0; s < shapes.size(); s++)
    {
        vertex_buffer_size += shapes[s].mesh.indices.size();
    }
    
    list_vertices.reserve(vertex_buffer_size);
    list_indices.reserve(vertex_buffer_size);
    
    size_t index_global_offset = 0;
    
    if (shapes.size() == 0 || shapes.size() > 1) Debug::LogError("Mesh not created successfully", false);
    
    for (size_t s = 0; s < shapes.size(); s++)
    {
        size_t index_offset = 0;
    
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++)
        {
            uint num_face_verts = shapes[s].mesh.num_face_vertices[f];
    
            for (uint v = 0; v < num_face_verts; v++)
            {
                tinyobj::index_t index = shapes[s].mesh.indices[index_offset + v];
    
                //adding vertex positions
                tinyobj::real_t vx = attribs.vertices[(int)(index.vertex_index * 3 + 0)];
                tinyobj::real_t vy = attribs.vertices[(int)(index.vertex_index * 3 + 1)];
                tinyobj::real_t vz = attribs.vertices[(int)(index.vertex_index * 3 + 2)];
    
                //adding texcoords
                tinyobj::real_t tx = 0;
                tinyobj::real_t ty = 0;
                if (attribs.texcoords.size())
                {
                    tx = attribs.texcoords[(int)(index.texcoord_index * 2 + 0)];
                    ty = 1.0f - attribs.texcoords[(int)(index.texcoord_index * 2 + 1)];
                }

                //adding normal
                Vector3 normal(0.0f);
                if (attribs.normals.size())
                {
                    normal.x = attribs.normals[(int)(index.normal_index * 3 + 0)];
                    normal.y = attribs.normals[(int)(index.normal_index * 3 + 1)];
                    normal.z = attribs.normals[(int)(index.normal_index * 3 + 2)];
                }

                VertexMesh vertex(Vector3(vx, vy, vz), Vector2(tx, ty), normal);
                list_vertices.push_back(vertex);
    
                list_indices.push_back((unsigned int)index_global_offset + v);
            }
    
            index_offset += num_face_verts;
            index_global_offset += num_face_verts;
        }
    }
    
    const VertexAttribute attribsList[] = {
        { 3 }, // numElements position attribute
        { 2 }, // numElements texture coordinates attribute
        { 3 }  // numElements normal attribute
    };
    
    m_vao = GraphicsEngine::GetInstance().createVertexArrayObject(
        // vertex buffer
        {
            (void*)&list_vertices[0],
            sizeof(VertexMesh),
            (uint)list_vertices.size(),
            (VertexAttribute*)attribsList,
            3
        },
        // index buffer
        {
            (void*)&list_indices[0],
            (uint)list_indices.size()
        }
        );

    m_instanceTransforms = desc.instanceTransforms; 
    initInstanceBuffer();
}

Mesh::~Mesh()
{
}

void Mesh::OnInspectorGUI()
{
    ImGui::Text(("Mesh Inspector - ID: " + m_uniqueID).c_str());
    ImGui::Separator();

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
                        m_instanceBufferDirty = true;
                    // Convert from radians to degrees for display

                    static Vector3 eulerAngles = transform.localRotation.ToEulerAngles().ToDegrees();
                    if (ImGui::DragFloat3("Rotation", eulerAngles.Data(), 0.1f))
                    {
                        // Convert the edited angles back to radians and update the quaternion
                        transform.localRotation = Quaternion(eulerAngles.ToRadians());
                        m_instanceBufferDirty = true;
                    }

                    if(ImGui::DragFloat3("Scale", transform.localScale.Data(), 0.1f))
                        m_instanceBufferDirty = true;

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

void Mesh::initInstanceBuffer()
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

int Mesh::getInstanceCount()
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