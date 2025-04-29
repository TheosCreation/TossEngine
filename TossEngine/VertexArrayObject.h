/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2025 Media Design School
File Name : VertexArrayObject.h
Description : VertexArrayObject class is a representation of a VAO to be used by the graphics engine class
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
**/

#pragma once
#include "Utils.h"

/**
 * @class VertexArrayObject
 * @brief A representation of a Vertex Array Object (VAO) to be used by the graphics engine class.
 */
class VertexArrayObject
{
public:
    /**
     * @brief Constructor for the VertexArrayObject class with vertex buffer data.
     * @param _data Description of the vertex buffer.
     */
    VertexArrayObject(const VertexBufferDesc& _data);

    /**
     * @brief Constructor for the VertexArrayObject class with vertex and index buffer data.
     * @param _vbDesc Description of the vertex buffer.
     * @param _ibDesc Description of the index buffer.
     */
    VertexArrayObject(const VertexBufferDesc& _vbDesc, const IndexBufferDesc& _ibDesc);

    /**
     * @brief Destructor for the VertexArrayObject class.
     */
    ~VertexArrayObject();

    /**
     * @brief Initializes the instance buffer for rendering instanced data.
     * @param instanceData Pointer to the instance data.
     * @param instanceCount Number of instances.
     */
    void initInstanceBuffer(Mat4* instanceData, size_t instanceCount);

    /**
     * @brief Gets the ID of the vertex array object.
     * @return The ID of the vertex array object.
     */
    uint getId();

    /**
     * @brief Gets the size of the vertex buffer.
     * @return The size of the vertex buffer.
     */
    uint getVertexBufferSize();

    /**
     * @brief Gets the number of indices in the index buffer.
     * @return The number of indices.
     */
    uint getNumIndices();

private:
    uint m_vertexBufferID = 0;              // The ID of the vertex buffer.
    uint m_elementBufferId = 0;             // The ID of the element (index) buffer.
    uint m_vertexArrayObjectID = 0;         // The ID of the vertex array object.
    uint m_instanceBufferID = 0;            // The ID of the instance buffer.
    VertexBufferDesc m_vertexBufferData;    // Description of the vertex buffer.
    IndexBufferDesc m_indexBufferDesc;      // Description of the index buffer.
};