#include "SkyboxEntity.h"
#include "GraphicsEngine.h"
#include "VertexArrayObject.h" 
#include "Mesh.h"
#include "Game.h"

void SkyboxEntity::onCreate()
{
    // This method can be used for initialization tasks when the skybox entity is created.
}

void SkyboxEntity::setUniformData(UniformData data)
{
    // This method can be implemented to set uniform data for shaders.
    // Currently, it does not perform any operations.
}

void SkyboxEntity::onGraphicsUpdate(UniformData data)
{
    // Call the base class graphics update to perform necessary updates
    GraphicsEntity::onGraphicsUpdate(data);

    // Get the graphics engine instance for rendering
    auto& graphicsEngine = GraphicsEngine::GetInstance();
    graphicsEngine.setShader(m_shader); // Set the shader for the skybox

    // Create a view matrix without translation for the skybox
    Mat4 viewNoTranslationMatrix = glm::mat3(data.viewMatrix);
    m_shader->setMat4("VPMatrix", data.projectionMatrix * viewNoTranslationMatrix); // Set the combined view-projection matrix

    // Configure graphics engine settings for rendering the skybox
    graphicsEngine.setFaceCulling(CullType::FrontFace); // Enable front face culling
    graphicsEngine.setDepthFunc(DepthType::LessEqual); // Set the depth function

    // Bind the skybox texture if available
    if (m_texture != nullptr)
    {
        graphicsEngine.setTextureCubeMap(m_texture, 0, "Texture_Skybox");
    }

    // Get the mesh's vertex array object and bind it to the graphics pipeline
    auto meshVBO = getMesh()->getVertexArrayObject();
    graphicsEngine.setVertexArrayObject(meshVBO);

    // Draw the indexed triangles for the skybox
    graphicsEngine.drawIndexedTriangles(TriangleType::TriangleList, meshVBO->getNumIndices());

    // Unbind the skybox texture
    graphicsEngine.setTextureCubeMap(nullptr, 0, "");
}

void SkyboxEntity::onGeometryPass(UniformData data)
{
    // Prepare for the geometry pass, setting up shaders and graphics engine
    auto& graphicsEngine = GraphicsEngine::GetInstance();
    graphicsEngine.setShader(m_geometryShader); // Set the geometry shader
    graphicsEngine.setFaceCulling(CullType::FrontFace); // Enable front face culling
    graphicsEngine.setDepthFunc(DepthType::LessEqual); // Set the depth function

    // Create a view matrix without translation for the geometry pass
    Mat4 viewNoTranslationMatrix = glm::mat3(data.viewMatrix);
    m_geometryShader->setMat4("VPMatrix", data.projectionMatrix * viewNoTranslationMatrix); // Set the combined view-projection matrix

    // Bind the skybox texture if available
    if (m_texture != nullptr)
    {
        m_geometryShader->setTextureCubeMap(m_texture, 0, "Texture_Skybox");
    }

    // Get the mesh's vertex array object and bind it to the graphics pipeline
    auto meshVBO = getMesh()->getVertexArrayObject();
    graphicsEngine.setVertexArrayObject(meshVBO);

    // Draw the indexed triangles for the geometry pass
    graphicsEngine.drawIndexedTriangles(TriangleType::TriangleList, meshVBO->getNumIndices());

    // Unbind the skybox texture
    m_geometryShader->setTextureCubeMap(nullptr, 0, "");
}