/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2024 Media Design School
File Name : InstancedMesh.h
Description : A class representing a instanced mesh resource
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
**/

#pragma once

#include "Mesh.h"
#include "Math.h"
#include <vector>

/**
 * @class InstancedMesh
 * @brief A class representing a instanced mesh resource.
 */
class InstancedMesh : public Mesh {
public:
    /**
     * @brief Constructor for the InstancedMesh class.
     * @param path The file path to the mesh resource.
     * @param manager Pointer to the resource manager.
     */
    InstancedMesh(const char* path, ResourceManager* manager);

    /**
     * @brief Destructor for the InstancedMesh class.
     */
    virtual ~InstancedMesh();

    /**
     * @brief Adds an instance of the mesh with the specified position, scale, and rotation.
     * @param Position The position of the instance.
     * @param Scale The scale of the instance.
     * @param Rotation The rotation of the instance.
     */
    void addInstance(Vector3 Position, Vector3 Scale, Vector3 Rotation);

    /**
     * @brief Initializes the instance buffer for rendering.
     */
    void initInstanceBuffer();

    /**
     * @brief Gets the count of instances.
     * @return The number of instances.
     */
    int getInstanceCount();

    /**
     * @brief Clears all the instances from the instance buffers and transform array
     */
    void clearInstances();

private:
    std::vector<Mat4> m_instanceTransforms; //A vector of transformation matrices for each instance.
};