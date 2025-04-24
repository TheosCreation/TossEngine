/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2024 Media Design School
File Name : Mesh.h
Description : A class representing a mesh resource
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
**/

#pragma once
#include <map>
#include <string>
#include "Utils.h"
#include "Resource.h"
#include "ResourceManager.h"

/**
 * @class Mesh
 * @brief A class representing a mesh resource.
 */
class TOSSENGINE_API Mesh : public Resource
{
public:
    /**
     * @brief Constructor for the Mesh class.
     * @param path The file path to the mesh resource.
     * @param manager Pointer to the resource manager.
     */
    Mesh(const MeshDesc& desc, const string& uniqueId, ResourceManager* manager);

    /**
     * @brief Destructor for the Mesh class.
     */
    virtual ~Mesh();

    void OnInspectorGUI() override;
    bool Delete(bool deleteSelf = true) override;

    /**
     * @brief Gets the Vertex Array Object (VAO) associated with this mesh.
     * @return A shared pointer to the Vertex Array Object.
     */
    VertexArrayObjectPtr getVertexArrayObject();

    void addInstance(Vector3 position, Vector3 scale, Vector3 rotation);

    void initInstanceBuffer() const;

    int getInstanceCount() const;
    std::vector<Transform> getInstanceTransforms() const;

    void clearInstances();

private:
    VertexArrayObjectPtr m_vao; // The Vertex Array Object associated with this mesh.
    std::vector<Transform> m_instanceTransforms; //A vector of transformations for each instance.

    Vector3 eulerAngles;
};

inline void to_json(json& j, MeshPtr const& mesh) {
    if (mesh)
    {
        j = json{ { "id", mesh->getUniqueID() } };
    }
    else {
        j = nullptr;
    }
}

inline void from_json(json const& j, MeshPtr& mesh) {
    if (j.contains("id")) mesh = ResourceManager::GetInstance().getMesh(j["id"].get<string>());
}