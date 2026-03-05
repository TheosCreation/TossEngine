#include "Skybox.h"
#include "GraphicsEngine.h"
#include "VertexArrayObject.h" 
#include "Material.h"
#include "TextureCubeMap.h"

void Skybox::onCreate()
{
    Renderer::onCreate();

    if (!m_mesh)
    {
        m_mesh = ResourceManager::GetInstance().get<Mesh>("Cube");
    }

    if (!m_material)
    {
        m_material = ResourceManager::GetInstance().get<Material>("SkyboxMaterial");
    }
}

void Skybox::Render(UniformData data, RenderingPath renderPath)
{
    if (!m_material || !m_mesh) return;
    m_material->setBinding("Texture_Skybox", m_texture);
    if(!m_material->Bind()) return;

    ShaderPtr shader = m_material->GetShader();
    
    // Get the graphics engine instance for rendering
   // graphicsEngine.setShader(shader); // Set the shader for the skybox

    // Create a view matrix without translation for the skybox
    Mat4 viewNoTranslationMatrix = Mat3(data.viewMatrix);
    shader->setMat4("VPMatrix", data.projectionMatrix * viewNoTranslationMatrix); // Set the combined view-projection matrix

    auto& graphicsEngine = GraphicsEngine::GetInstance();
    // Configure graphics engine settings for rendering the skybox
    graphicsEngine.setFaceCulling(CullType::FrontFace); // Enable front face culling
    graphicsEngine.setWindingOrder(WindingOrder::CounterClockWise);

    // Get the mesh's vertex array object and bind it to the graphics pipeline
    auto meshVBO = m_mesh->getVertexArrayObject();
    if (!meshVBO) return;
    graphicsEngine.setVertexArrayObject(meshVBO);

    // Draw the indexed triangles for the skybox
    graphicsEngine.drawIndexedTriangles(TriangleType::TriangleList, meshVBO->getNumIndices());
}

void Skybox::setMesh(const MeshPtr& mesh)
{
    m_mesh = mesh;
}

void Skybox::setTextureCubeMap(const shared_ptr<TextureCubeMap>& texture)
{
    m_texture = texture;
}
