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
class Mesh : public Resource
{
public:
    /**
     * @brief Constructor for the Mesh class.
     * @param path The file path to the mesh resource.
     * @param manager Pointer to the resource manager.
     */
    Mesh(const char* path, ResourceManager* manager);

    /**
     * @brief Destructor for the Mesh class.
     */
    virtual ~Mesh();

    /**
     * @brief Gets the Vertex Array Object (VAO) associated with this mesh.
     * @return A shared pointer to the Vertex Array Object.
     */
    VertexArrayObjectPtr getVertexArrayObject();

private:
    VertexArrayObjectPtr m_vao; // The Vertex Array Object associated with this mesh.
};