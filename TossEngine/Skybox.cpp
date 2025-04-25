#include "Skybox.h"
#include "GraphicsEngine.h"
#include "VertexArrayObject.h" 
#include "Mesh.h"
#include "Material.h"
#include "TextureCubeMap.h"

json Skybox::serialize() const
{
    json data;
    data["type"] = getClassName(typeid(*this)); // Store the component type

    // Serialize mesh
    if (m_mesh)
    {
        data["mesh"] = m_mesh->getUniqueID();
    }

    if (m_material)
    {
        data["material"] = m_material->getUniqueID();
    }

    return data;
}

void Skybox::deserialize(const json& data)
{
    auto& resourceManager = ResourceManager::GetInstance();

    // Deserialize mesh
    if (data.contains("mesh"))
    {
        std::string meshId = data["mesh"];
        m_mesh = resourceManager.getMesh(meshId);
    }
    
    // Deserialize material
    if (data.contains("material"))
    {
        std::string materialName = data["material"];
        m_material = resourceManager.getMaterial(materialName);
    }
}

void Skybox::onCreate()
{
    // This method can be used for initialization tasks when the skybox GameObject is created.
}

void Skybox::Render(UniformData data, RenderingPath renderPath)
{
    if (m_material == nullptr || m_mesh == nullptr) return;
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
    graphicsEngine.setDepthFunc(DepthType::LessEqual); // Set the depth function

    // Bind the skybox texture if available
    //if (m_texture != nullptr)
    //{
    //    graphicsEngine.setTextureCubeMap(m_texture, 0, "Texture_Skybox");
    //}

    // Get the mesh's vertex array object and bind it to the graphics pipeline
    auto meshVBO = m_mesh->getVertexArrayObject();
    if (!meshVBO) return;
    graphicsEngine.setVertexArrayObject(meshVBO);

    // Draw the indexed triangles for the skybox
    graphicsEngine.drawIndexedTriangles(TriangleType::TriangleList, meshVBO->getNumIndices());
}

void Skybox::setMesh(MeshPtr mesh)
{
    m_mesh = mesh;
}
