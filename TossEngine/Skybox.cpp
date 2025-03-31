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

    // Serialize textures
    if (m_texture)
    {
        data["texture"] = m_texture->getUniqueID();
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

    // Deserialize textures
    if (data.contains("texture"))
    {
        std::string textureName = data["texture"];
        m_texture = resourceManager.getCubemapTexture(textureName);
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
    if (m_material == nullptr || m_mesh == nullptr || m_texture == nullptr) return;
    //m_material->Bind();

    ShaderPtr shader = m_material->GetShader();
    
    // Get the graphics engine instance for rendering
    auto& graphicsEngine = GraphicsEngine::GetInstance();
    graphicsEngine.setShader(shader); // Set the shader for the skybox

    // Create a view matrix without translation for the skybox
    Mat4 viewNoTranslationMatrix = Mat3(data.viewMatrix);
    shader->setMat4("VPMatrix", data.projectionMatrix * viewNoTranslationMatrix); // Set the combined view-projection matrix

    // Configure graphics engine settings for rendering the skybox
    graphicsEngine.setFaceCulling(CullType::FrontFace); // Enable front face culling
    graphicsEngine.setWindingOrder(WindingOrder::CounterClockWise);
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
}

TexturePtr Skybox::getTexture()
{
    return m_texture;
}

void Skybox::setTexture(TexturePtr texture)
{
    m_texture = texture;
}

void Skybox::setMesh(MeshPtr mesh)
{
    m_mesh = mesh;
}
