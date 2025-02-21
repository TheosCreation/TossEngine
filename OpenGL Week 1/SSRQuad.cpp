#include "SSRQuad.h"
#include "LightManager.h"
#include "GeometryBuffer.h"
#include "ResourceManager.h"
#include "GraphicsEngine.h"
#include "VertexArrayObject.h"

void SSRQuad::onGraphicsUpdate(UniformData data)
{
    // Set the shader for the lighting pass
    auto& graphicsEngine = GraphicsEngine::GetInstance();
    graphicsEngine.setShader(m_shader);

    if (m_texture)
    {
        m_shader->setTexture2D(m_texture, 0, "Texture0");
    }

    if (m_texture1)
    {
        m_shader->setTexture2D(m_texture1, 1, "Texture1");
    }

    // Prepare the graphics engine for rendering
    graphicsEngine.setFaceCulling(CullType::None); // Disable face culling
    graphicsEngine.setWindingOrder(WindingOrder::ClockWise); // Set winding order
    graphicsEngine.setVertexArrayObject(m_mesh); // Bind the vertex array object for the mesh
    graphicsEngine.drawIndexedTriangles(TriangleType::TriangleList, m_mesh->getNumIndices()); // Draw the indexed triangles
}

void SSRQuad::onLightingPass(UniformData data)
{
    // Set the shader for the lighting pass
    auto& graphicsEngine = GraphicsEngine::GetInstance();
    graphicsEngine.setShader(m_lightingShader);

    // Populate the shader with geometry buffer information for the lighting pass
    GeometryBuffer::GetInstance().PopulateShader(m_lightingShader);

    // Apply lighting settings using the LightManager
    LightManager::GetInstance().applyLighting(m_lightingShader);

    // Set the camera position in the lighting shader
    m_lightingShader->setVec3("CameraPos", data.cameraPosition);

    auto skyboxTexture = ResourceManager::GetInstance().getSkyboxTexture();
    if (skyboxTexture)
    {
        m_lightingShader->setTextureCubeMap(skyboxTexture, 5, "Texture_Skybox");
    }

    // Apply shadows if available
    auto& lightManager = LightManager::GetInstance();
    lightManager.applyShadows(m_lightingShader);

    // Prepare the graphics engine for rendering
    graphicsEngine.setFaceCulling(CullType::None); // Disable face culling
    graphicsEngine.setWindingOrder(WindingOrder::ClockWise); // Set winding order
    graphicsEngine.setVertexArrayObject(m_mesh); // Bind the vertex array object for the mesh
    graphicsEngine.drawIndexedTriangles(TriangleType::TriangleList, m_mesh->getNumIndices()); // Draw the indexed triangles
}

void SSRQuad::onShadowPass(uint index)
{
}