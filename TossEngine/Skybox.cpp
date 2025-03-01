#include "Skybox.h"
#include "GraphicsEngine.h"
#include "VertexArrayObject.h" 
#include "Mesh.h"
#include "Game.h"
#include "Material.h"

void Skybox::onCreate()
{
    // This method can be used for initialization tasks when the skybox GameObject is created.
}

void Skybox::Render(UniformData data, RenderingPath renderPath)
{
    ShaderPtr shader = m_material->GetShader();
    
    // Get the graphics engine instance for rendering
    auto& graphicsEngine = GraphicsEngine::GetInstance();
    graphicsEngine.setShader(shader); // Set the shader for the skybox

    // Create a view matrix without translation for the skybox
    Mat4 viewNoTranslationMatrix = glm::mat3(data.viewMatrix);
    shader->setMat4("VPMatrix", data.projectionMatrix * viewNoTranslationMatrix); // Set the combined view-projection matrix

    // Configure graphics engine settings for rendering the skybox
    graphicsEngine.setFaceCulling(CullType::FrontFace); // Enable front face culling
    graphicsEngine.setDepthFunc(DepthType::LessEqual); // Set the depth function

    // Bind the skybox texture if available
    if (m_texture != nullptr)
    {
        graphicsEngine.setTextureCubeMap(m_texture, 0, "Texture_Skybox");
    }

    // Get the mesh's vertex array object and bind it to the graphics pipeline
    auto meshVBO = m_mesh->getVertexArrayObject();
    graphicsEngine.setVertexArrayObject(meshVBO);

    // Draw the indexed triangles for the skybox
    graphicsEngine.drawIndexedTriangles(TriangleType::TriangleList, meshVBO->getNumIndices());

    // Unbind the skybox texture
    graphicsEngine.setTextureCubeMap(nullptr, 0, "");
}

void Skybox::setTexture(TexturePtr texture)
{
    m_texture = texture;
}

void Skybox::setMesh(MeshPtr mesh)
{
    m_mesh = mesh;
}
