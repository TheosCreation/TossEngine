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
    Mesh(const string& filePath, ResourceManager* manager);

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

    void initInstanceBuffer();

    int getInstanceCount();

    void clearInstances();

private:
    VertexArrayObjectPtr m_vao; // The Vertex Array Object associated with this mesh.
    std::vector<Transform> m_instanceTransforms; //A vector of transformations for each instance.
};