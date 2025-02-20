#include "SSRQuad.h"
#include "LightManager.h"
#include "GeometryBuffer.h"
#include "ResourceManager.h"
#include "GraphicsEngine.h"
#include "VertexArrayObject.h"

void SSRQuad::onGraphicsUpdate(UniformData data)
{
    // Call the base class's graphics update method
    GraphicsEntity::onGraphicsUpdate(data);

    // Populate the shader with geometry buffer information
    GeometryBuffer::GetInstance().PopulateShader(m_shader);

    // Set up the graphics engine for rendering
    auto& graphicsEngine = GraphicsEngine::GetInstance();
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

    // Apply lighting settings using the LightManager
    LightManager::GetInstance().applyLighting(m_lightingShader);

    // Populate the shader with geometry buffer information for the lighting pass
    GeometryBuffer::GetInstance().PopulateShader(m_lightingShader);

    // Set the camera position in the lighting shader
    m_lightingShader->setVec3("CameraPos", data.cameraPosition);

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
    // Set the shader for the shadow pass
    auto& graphicsEngine = GraphicsEngine::GetInstance();
    graphicsEngine.setShader(m_shadowShader);

    // Set the light space matrix for shadow mapping
    m_shadowShader->setMat4("VPLight", LightManager::GetInstance().getLightSpaceMatrix(index));

    // Prepare the graphics engine for rendering shadows
    graphicsEngine.setFaceCulling(CullType::None); // Disable face culling
    graphicsEngine.setWindingOrder(WindingOrder::ClockWise); // Set winding order
    graphicsEngine.setVertexArrayObject(m_mesh); // Bind the vertex array object for the mesh
    graphicsEngine.drawIndexedTriangles(TriangleType::TriangleList, m_mesh->getNumIndices()); // Draw the indexed triangles
}