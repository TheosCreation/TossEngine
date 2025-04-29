/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2024 Media Design School
File Name : Mesh.h
Description : A class representing a mesh resource, supporting instanced rendering.
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
***/

#pragma once

#include <map>
#include <string>
#include "Utils.h"
#include "Resource.h"
#include "ResourceManager.h"

/**
 * @class Mesh
 * @brief A class representing a mesh resource containing geometry data and supporting instancing.
 */
class TOSSENGINE_API Mesh : public Resource
{
public:
    /**
     * @brief Constructor for creating a mesh resource from a description.
     * @param desc The mesh description (path, vertices, indices, etc.).
     * @param uniqueId The unique resource ID.
     * @param manager Pointer to the resource manager.
     */
    Mesh(const MeshDesc& desc, const std::string& uniqueId, ResourceManager* manager);

    /**
     * @brief Constructor for loading an existing mesh resource.
     * @param uid The unique resource ID.
     * @param mgr Pointer to the resource manager.
     */
    Mesh(const std::string& uid, ResourceManager* mgr);

    /**
     * @brief Final initialization after resource creation.
     */
    void onCreateLate() override;

    /**
     * @brief Loads mesh data from a file path.
     */
    void LoadMeshFromFilePath();

    /**
     * @brief Destructor for the Mesh class.
     */
    virtual ~Mesh();

    /**
     * @brief Draws the inspector GUI for editing the mesh resource.
     */
    void OnInspectorGUI() override;

    /**
     * @brief Deletes the mesh resource.
     * @param deleteSelf Whether to remove it from the resource manager.
     * @return True if deleted, false otherwise.
     */
    bool Delete(bool deleteSelf = true) override;

    /**
     * @brief Gets the Vertex Array Object (VAO) associated with this mesh.
     * @return Shared pointer to the Vertex Array Object.
     */
    VertexArrayObjectPtr getVertexArrayObject();

    /**
     * @brief Adds an instance of the mesh with a given transform.
     * @param position Position of the instance.
     * @param scale Scale of the instance.
     * @param rotation Euler rotation of the instance.
     */
    void addInstance(Vector3 position, Vector3 scale, Vector3 rotation);

    /**
     * @brief Initializes or updates the instance buffer for instanced rendering.
     */
    void initInstanceBuffer() const;

    /**
     * @brief Gets the number of mesh instances.
     * @return Number of instances.
     */
    int getInstanceCount() const;

    /**
     * @brief Gets the transforms of all mesh instances.
     * @return A vector of transforms.
     */
    std::vector<Transform> getInstanceTransforms() const;

    /**
     * @brief Clears all mesh instances.
     */
    void clearInstances();

private:
    VertexArrayObjectPtr m_vao; //!< The Vertex Array Object associated with this mesh.
    std::vector<Transform> m_instanceTransforms; //!< Transformations for each mesh instance.
    Vector3 eulerAngles; //!< Euler rotation angles used internally.

    SERIALIZABLE_MEMBERS(m_path, m_instanceTransforms)
};

REGISTER_RESOURCE(Mesh)

// --- JSON Serialization ---

inline void to_json(json& j, const MeshPtr& mesh) {
    if (mesh)
    {
        j = json{ { "id", mesh->getUniqueID() } };
    }
    else {
        j = nullptr;
    }
}

inline void from_json(const json& j, MeshPtr& mesh) {
    if (j.contains("id") && !j["id"].is_null())
        mesh = ResourceManager::GetInstance().get<Mesh>(j["id"].get<std::string>());
}