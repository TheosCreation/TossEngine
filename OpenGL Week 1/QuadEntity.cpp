/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2024 Media Design School
File Name : QuadEntity.cpp
Description : Entity type that renders a quad
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
**/

#include "QuadEntity.h"
#include "GraphicsEngine.h"
#include "Game.h"
#include "VertexArrayObject.h"

void QuadEntity::onCreate()
{
    updateVertices({ -2.0f, 2.0f });
}

void QuadEntity::updateVertices(Vector2 size)
{
    Vector3 position_list[] =
    {
        //front face
        { Vector3(size.x / 2, -size.y / 2, 0.5f) },
        { Vector3(size.x / 2, size.y / 2, 0.5f) },
        { Vector3(-size.x / 2, size.y / 2, 0.5f)},
        { Vector3(-size.x / 2, -size.y / 2, 0.5f) }
    };

    glm::vec2 texcoord_list[] =
    {
        { glm::vec2(0.0f,1.0f) },
        { glm::vec2(0.0f,0.0f) },
        { glm::vec2(1.0f,1.0f) },
        { glm::vec2(1.0f,0.0f) }
    };

    Vertex verticesList[] =
    {
        //front face
        { position_list[0],texcoord_list[1] },
        { position_list[1],texcoord_list[0] },
        { position_list[2],texcoord_list[2] },
        { position_list[3],texcoord_list[3] }
    };

    uint indicesList[] =
    {
        //front face
        0,1,2,  //first triangle
        2,3,0  //second triangle
    };

    static const VertexAttribute attribsList[] = {
        { 3 }, //numElements position attribute
        { 2 } //numElements texture coordinates attribute
    };

    m_mesh = GraphicsEngine::GetInstance().createVertexArrayObject(
        //vertex buffer
        {
                (void*)verticesList,
                sizeof(Vertex), //size in bytes of a single composed vertex (in this case composed by vertex (3 nums* sizeof float) + texcoord (2 nums* sizeof float))
                sizeof(verticesList) / sizeof(Vertex),  //number of composed vertices,

                (VertexAttribute*)attribsList,
                2 //numelements attrib list
        },
        //index buffer
        {
            (void*)indicesList,
            sizeof(indicesList) / sizeof(uint)
        }
    );
}

void QuadEntity::setUniformData(UniformData data)
{
}

void QuadEntity::setShader(const ShaderPtr& shader)
{
    m_shader = shader;
}

void QuadEntity::onGraphicsUpdate(UniformData data)
{
    GraphicsEntity::onGraphicsUpdate(data);

    m_shader->setMat4("VPMatrix", data.projectionMatrix * data.viewMatrix);
    m_shader->setMat4("modelMatrix", m_transform.GetMatrix());

    auto& graphicsEngine = GraphicsEngine::GetInstance();
    if (m_texture != nullptr)
    {
        graphicsEngine.setTexture2D(m_texture, 0, "Texture0");
    }
    if (m_texture1 != nullptr)
    {
        graphicsEngine.setTexture2D(m_texture1, 1, "Texture1");
    }
    if (m_texture2 != nullptr)
    {
        graphicsEngine.setTexture2D(m_texture2, 2, "Texture2");
    }
    if (m_texture3 != nullptr)
    {
        graphicsEngine.setTexture2D(m_texture3, 3, "Texture3");
    }
    if (m_heightMap != nullptr)
    {
        graphicsEngine.setTexture2D(m_heightMap, 4, "HeightMap");
    }
    graphicsEngine.setFaceCulling(CullType::None);
    graphicsEngine.setWindingOrder(WindingOrder::ClockWise);
    graphicsEngine.setVertexArrayObject(m_mesh); //bind vertex buffer to graphics pipeline
    graphicsEngine.drawIndexedTriangles(TriangleType::TriangleList, m_mesh->getNumIndices());//draw triangles through the usage of index buffer

    graphicsEngine.setTexture2D(nullptr, 0, "");
    graphicsEngine.setTexture2D(nullptr, 1, "");
    graphicsEngine.setTexture2D(nullptr, 2, "");
    graphicsEngine.setTexture2D(nullptr, 3, "");
    graphicsEngine.setTexture2D(nullptr, 4, "");
}

void QuadEntity::onGraphicsUpdate(NewUniformData& _data)
{
    NewExtraTextureData _textureData;
    onGraphicsUpdate(_data, _textureData);
}

void QuadEntity::onGraphicsUpdate(NewUniformData& _data, NewExtraTextureData& _textureData)
{
    UniformData data = {};
    GraphicsEntity::onGraphicsUpdate(data); 

    for (const auto& [key, value] : _data.dataMap) {
        ProcessUniformData(key, value);
    }
    
    for (const auto& [key, value] : _textureData.textureMap) {
        ProcessTextureData(key, value);
    }

    auto& graphicsEngine = GraphicsEngine::GetInstance();
    if (m_texture != nullptr)
    {
        graphicsEngine.setTexture2D(m_texture, 0, "Texture0");
    }
    graphicsEngine.setFaceCulling(CullType::None);
    graphicsEngine.setWindingOrder(WindingOrder::ClockWise);
    graphicsEngine.setVertexArrayObject(m_mesh); //bind vertex buffer to graphics pipeline
    graphicsEngine.drawIndexedTriangles(TriangleType::TriangleList, m_mesh->getNumIndices());//draw triangles through the usage of index buffer

    graphicsEngine.setTexture2D(nullptr, 0, "");
    graphicsEngine.setTexture2D(nullptr, 1, "");
}

void QuadEntity::setTextureFromId(uint textureId)
{
    m_texture = std::make_shared<Texture>();
    m_texture->setId(textureId);
}
