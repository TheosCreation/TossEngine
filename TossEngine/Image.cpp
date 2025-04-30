#include "Image.h"
#include "GraphicsEngine.h"
#include "VertexArrayObject.h"
#include "Material.h"
#include "Shader.h"
#include "GameObject.h"

void Image::onCreate()
{
    updateVertices();
}
Rect Image::getWorldRect()
{
    // Define the four corners of the rectangle in local space
    Vector3 localCorners[4] = {
        Vector3(-m_size.x / 2, -m_size.y / 2, 0.0f), // Bottom-left
        Vector3(-m_size.x / 2,  m_size.y / 2, 0.0f), // Top-left
        Vector3(m_size.x / 2,   m_size.y / 2, 0.0f), // Top-right
        Vector3(m_size.x / 2,  -m_size.y / 2, 0.0f)  // Bottom-right
    };

    // Transform the local corners to world space
    Vector3 worldCorners[4];
    Mat4 worldMatrix = m_owner->m_transform.GetMatrix();
    for (int i = 0; i < 4; ++i)
    {
        worldCorners[i] = worldMatrix.TransformPoint(localCorners[i]);
    }

    // Initialize min/max
    float minX = worldCorners[0].x;
    float maxX = worldCorners[0].x;
    float minY = worldCorners[0].y;
    float maxY = worldCorners[0].y;

    for (int i = 1; i < 4; ++i)
    {
        if (worldCorners[i].x < minX) minX = worldCorners[i].x;
        if (worldCorners[i].x > maxX) maxX = worldCorners[i].x;
        if (worldCorners[i].y < minY) minY = worldCorners[i].y;
        if (worldCorners[i].y > maxY) maxY = worldCorners[i].y;
    }

    // Return Rect: top-left and size
    // Important: keep top as maxY to match "top-left" expectations
    return Rect(Vector2(minX, maxY), Vector2(maxX - minX, maxY - minY));
}

Vector2 Image::GetPivotOffsetFromCenter() const
{
    float halfWidth = m_size.x * 0.5f;
    float halfHeight = m_size.y * 0.5f;

    float x = 0.0f;
    float y = 0.0f;

    switch (m_pivotPoint)
    {
    case AnchorPoint::TopLeft:      x = -halfWidth; y = halfHeight; break;
    case AnchorPoint::Center:       x = 0.0f;       y = 0.0f;       break;
    case AnchorPoint::BottomRight:  x = halfWidth; y = -halfHeight; break;
    }

    return Vector2(x, y);
}


void Image::updateVertices()
{
    Vector3 position_list[] =
    {
        //front face
        { Vector3(m_size.x / 2,  -m_size.y / 2, 0.0f) },
        { Vector3(m_size.x / 2,   m_size.y / 2, 0.0f) },
        { Vector3(-m_size.x / 2,   m_size.y / 2, 0.0f) },
        { Vector3(-m_size.x / 2,  -m_size.y / 2, 0.0f) }
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
    m_vao = GraphicsEngine::GetInstance().createVertexArrayObject(
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
    if (!m_material) return;

    ShaderPtr shader = m_material->GetShader();
    // Set the shader for the lighting pass
    auto& graphicsEngine = GraphicsEngine::GetInstance();
    graphicsEngine.setShader(shader);

    //have to check if it has an owner because image is used from drawing the scene and game view windows
    if (m_owner)
    {
        Mat4 modelMatrix;

        if (m_isUi)
        {
            Vector2 screenSize = data.uiScreenSize;
            Vector2 anchorOffset = GetAnchorOffset(screenSize, m_size, m_anchorPoint);
            Vector2 pivotOffset = GetPivotOffsetFromCenter();

            // Build base model matrix from the object's transform
            modelMatrix = m_owner->m_transform.GetMatrix();

            // Apply anchor offset (position relative to screen)
            modelMatrix = Mat4::Translate(Vector3(anchorOffset.x, -anchorOffset.y, 0.0f)) * modelMatrix;

            // Then apply pivot offset relative to size
            //modelMatrix = Mat4::Translate(Vector3(-pivotOffset.x, -pivotOffset.y, 0.0f)) * modelMatrix;

            shader->setMat4("modelMatrix", modelMatrix);
            shader->setMat4("VPMatrix", data.uiProjectionMatrix);
        }
        else
        {
            modelMatrix = m_owner->m_transform.GetMatrix();

            if (m_pivotPoint != AnchorPoint::Center)
            {
                Vector2 pivotOffset = GetPivotOffsetFromCenter();
                modelMatrix = modelMatrix * Mat4::Translate(Vector3(-pivotOffset.x, -pivotOffset.y, 0.0f));
            }

            shader->setMat4("modelMatrix", modelMatrix);
            shader->setMat4("VPMatrix", data.projectionMatrix * data.viewMatrix);
        }
    }

    if (m_texture)
    {
        shader->setTexture2D(m_texture, 0, "Texture0");
    }
    
    // Prepare the graphics engine for rendering
    graphicsEngine.setFaceCulling(CullType::None); // Disable face culling
    graphicsEngine.setWindingOrder(WindingOrder::ClockWise); // Set winding order
    graphicsEngine.setVertexArrayObject(m_vao); // Bind the vertex array object for the mesh
    graphicsEngine.drawIndexedTriangles(TriangleType::TriangleList, m_vao->getNumIndices()); // Draw the indexed triangles
}


void Image::SetSize(Vector2 size)
{
    if (m_size != size)
    {
        m_size = size;
        updateVertices(); // Update the vertices with the new size
    }
}

void Image::SetTexture(const Texture2DPtr& texture)
{
    m_texture = texture;
}
