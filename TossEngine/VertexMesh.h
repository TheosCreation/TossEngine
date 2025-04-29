/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2025 Media Design School
File Name : VertexMesh.h
Description : VertexMesh is a standard representation for a mesh vertex.
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
**/

#pragma once
#include "Math.h"

/**
 * @class VertexMesh
 * @brief A standard representation for a mesh vertex.
 */
class VertexMesh
{
public:
    /**
     * @brief Default constructor for the VertexMesh class.
     */
    VertexMesh() : m_position(), m_texcoord(), m_normal()
    {
    }

    /**
     * @brief Constructor for the VertexMesh class with position, texture coordinate, and normal.
     * @param position The position of the vertex.
     * @param texcoord The texture coordinate of the vertex.
     * @param normal The normal vector of the vertex.
     */
    VertexMesh(const Vector3& _position, const glm::vec2& _texcoord, const Vector3& _normal) :
        m_position(_position),
        m_texcoord(_texcoord),
        m_normal(_normal)
    {
    }

    /**
     * @brief Copy constructor for the VertexMesh class.
     * @param vertex The vertex to copy.
     */
    VertexMesh(const VertexMesh& _vertex) :
        m_position(_vertex.m_position),
        m_texcoord(_vertex.m_texcoord),
        m_normal(_vertex.m_normal)
    {
    }

public:
    Vector3 m_position;     // The position of the vertex.
    glm::vec2 m_texcoord;   // The texture coordinate of the vertex.
    Vector3 m_normal;       // The normal vector of the vertex.
};