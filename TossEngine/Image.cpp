#include "Image.h"
#include "GraphicsEngine.h"
#include "VertexArrayObject.h"
#include "Material.h"
#include "Shader.h"

void Image::onCreate()
{
    updateVertices();
}

void Image::updateVertices()
{
    Vector3 position_list[] =
    {
        //front face
        { Vector3(m_size.x / 2, -m_size.y / 2, 0.5f) },
        { Vector3(m_size.x / 2, m_size.y / 2, 0.5f) },
        { Vector3(-m_size.x / 2, m_size.y / 2, 0.5f)},
        { Vector3(-m_size.x / 2, -m_size.y / 2, 0.5f) }
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

    // Create a new VBO
    m_vbo = GraphicsEngine::GetInstance().createVertexArrayObject(
        //vertex buffer
        {
                (void*)verticesList,
                sizeof(Vertex), //size in bytes of a single composed vertex
                sizeof(verticesList) / sizeof(Vertex),  //number of composed vertices
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

void Image::Render(UniformData data, RenderingPath renderPath)
{
    ShaderPtr shader = m_material->GetShader();
    // Set the shader for the lighting pass
    auto& graphicsEngine = GraphicsEngine::GetInstance();
    graphicsEngine.setShader(shader);

    if (m_texture)
    {
        shader->setTexture2D(m_texture, 0, "Texture0");
    }
    
    // Prepare the graphics engine for rendering
    graphicsEngine.setFaceCulling(CullType::None); // Disable face culling
    graphicsEngine.setWindingOrder(WindingOrder::ClockWise); // Set winding order
    graphicsEngine.setVertexArrayObject(m_vbo); // Bind the vertex array object for the mesh
    graphicsEngine.drawIndexedTriangles(TriangleType::TriangleList, m_vbo->getNumIndices()); // Draw the indexed triangles
}


void Image::SetSize(Vector2 size)
{
    if (m_size != size)
    {
        m_size = size;
        updateVertices(); // Update the vertices with the new size
    }
}

void Image::SetTexture(const TexturePtr& texture)
{
    m_texture = texture;
}
