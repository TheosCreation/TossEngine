#include "MeshRenderer.h"
#include "ResourceManager.h"
#include "GraphicsEngine.h"
#include "LightManager.h"
#include "Mesh.h"
#include "Shader.h"
#include "GameObject.h"
#include "GameObjectManager.h"
#include "Texture.h"
#include "TextureCubeMap.h"
#include "VertexArrayObject.h"

json MeshRenderer::serialize() const
{
    json data;
    data["type"] = getClassName(typeid(*this)); // Store the component type

    // Serialize mesh
    if (m_mesh)
    {
        data["mesh"] = m_mesh->getUniqueID();
    }

    // Serialize shaders
    if (m_shader)
    {
        data["shader"] = m_shader->getUniqueID();
    }
    if (m_geometryShader)
    {
        data["geometryShader"] = m_geometryShader->getUniqueID();
    }
    if (m_shadowShader)
    {
        data["shadowShader"] = m_shadowShader->getUniqueID();
    }

    // Serialize textures
    if (m_texture)
    {
        data["texture"] = m_texture->getUniqueID();
    }
    if (m_reflectiveMap)
    {
        data["reflectiveMap"] = m_reflectiveMap->getUniqueID();
    }

    // Serialize material properties
    data["shininess"] = m_shininess;
    data["alpha"] = m_alpha;
    data["color"] = { m_color.x, m_color.y, m_color.z };

    return data;
}

void MeshRenderer::deserialize(const json& data)
{
    auto& resourceManager = ResourceManager::GetInstance();

    // Deserialize mesh
    if (data.contains("mesh"))
    {
        std::string meshId = data["mesh"];
        m_mesh = resourceManager.getMesh(meshId);
    }

    // Deserialize shaders
    if (data.contains("shader"))
    {
        std::string shaderId = data["shader"];
        m_shader = resourceManager.getShader(shaderId);
    }
    if (data.contains("geometryShader"))
    {
        std::string geometryId = data["geometryShader"];
        m_geometryShader = resourceManager.getShader(geometryId);
    }
    if (data.contains("shadowShader"))
    {
        std::string shadowShaderId = data["shadowShader"];
        m_shadowShader = resourceManager.getShader(shadowShaderId);
    }

    // Deserialize textures
    if (data.contains("texture"))
    {
        std::string textureName = data["texture"];
        m_texture = resourceManager.getTexture2D(textureName); // Replace with appropriate method to load texture
    }
    if (data.contains("reflectiveMap"))
    {
        std::string reflectiveMapName = data["reflectiveMap"];
        m_reflectiveMap = resourceManager.getTexture2D(reflectiveMapName);
    }

    // Deserialize material properties
    if (data.contains("shininess"))
    {
        m_shininess = data["shininess"];
    }
    if (data.contains("alpha"))
    {
        m_alpha = data["alpha"];
    }
    if (data.contains("color"))
    {
        auto color = data["color"];
        m_color = Vector3(color[0], color[1], color[2]);
    }
}
void MeshRenderer::onShadowPass(uint index)
{
    if (m_shadowShader == nullptr || m_mesh == nullptr) return;

    auto& graphicsEngine = GraphicsEngine::GetInstance();
    graphicsEngine.setFaceCulling(CullType::BackFace);
    graphicsEngine.setWindingOrder(WindingOrder::CounterClockWise);
    graphicsEngine.setDepthFunc(DepthType::Less);
    graphicsEngine.setShader(m_shadowShader);

    m_shadowShader->setMat4("VPLight", m_owner->getLightManager()->getLightSpaceMatrix(index));
    m_shadowShader->setMat4("modelMatrix", m_owner->m_transform.GetMatrix());

    // Bind the vertex array object for the mesh
    auto meshVBO = m_mesh->getVertexArrayObject();
    if (meshVBO == nullptr) return;

    graphicsEngine.setVertexArrayObject(meshVBO);

    // Draw the mesh to update the shadow map
    if (m_mesh->getInstanceCount() > 0)
    {
        // mesh has instances so we render instanced
        graphicsEngine.drawIndexedTrianglesInstanced(TriangleType::TriangleList, meshVBO->getNumIndices(), m_mesh->getInstanceCount());
    }
    else
    {
        graphicsEngine.drawIndexedTriangles(TriangleType::TriangleList, meshVBO->getNumIndices());
    }
}

void MeshRenderer::Render(UniformData data, RenderingPath renderPath)
{
    if (m_mesh == nullptr) return;

    auto& graphicsEngine = GraphicsEngine::GetInstance();
    graphicsEngine.setFaceCulling(CullType::BackFace);
    graphicsEngine.setWindingOrder(WindingOrder::CounterClockWise);
    graphicsEngine.setDepthFunc(DepthType::LessEqual);

    if (renderPath == RenderingPath::Deferred)
    {
        if (m_geometryShader == nullptr) return;

        graphicsEngine.setShader(m_geometryShader);
        m_geometryShader->setMat4("VPMatrix", data.projectionMatrix * data.viewMatrix);
        m_geometryShader->setMat4("modelMatrix", m_owner->m_transform.GetMatrix());
        m_geometryShader->setFloat("ObjectShininess", m_shininess);

        if (m_texture != nullptr)
        {
            m_geometryShader->setTexture2D(m_texture, 0, "Texture0");
            m_geometryShader->setBool("useTexture", true);
        }
        else
        {
            m_geometryShader->setVec3("uColor", m_color);
            m_geometryShader->setBool("useTexture", false);
        }

        if (m_reflectiveMap)
        {
            m_geometryShader->setTexture2D(m_reflectiveMap, 1, "ReflectionMap");
        }
    }

    if (renderPath == RenderingPath::Forward)
    {
        if (m_shader == nullptr) return;

        graphicsEngine.setShader(m_shader);
        m_shader->setMat4("VPMatrix", data.projectionMatrix * data.viewMatrix);
        m_shader->setMat4("modelMatrix", m_owner->m_transform.GetMatrix());
        m_shader->setVec3("CameraPos", data.cameraPosition);
        m_shader->setFloat("ObjectShininess", m_shininess);

        m_owner->getLightManager()->applyLighting(m_shader);

        if (m_texture != nullptr)
        {
            m_shader->setTexture2D(m_texture, 0, "Texture0");
            m_shader->setBool("useTexture", true);
            m_shader->setFloat("alpha", m_alpha);
        }
        else
        {
            m_shader->setVec3("uColor", m_color);
            m_shader->setBool("useTexture", false);
            m_shader->setFloat("alpha", m_alpha);
        }

        auto skyboxTexture = m_owner->getGameObjectManager()->getSkyBoxTexture();
        if (skyboxTexture)
        {
            graphicsEngine.setTextureCubeMap(skyboxTexture, 6, "Texture_Skybox");
        }

        if (m_reflectiveMap)
        {
            graphicsEngine.setTexture2D(m_reflectiveMap, 1, "ReflectionMap");
        }

        m_owner->getLightManager()->applyShadows(m_shader);
    }

    // Bind the vertex array object for the mesh
    auto meshVBO = m_mesh->getVertexArrayObject();
    if (meshVBO == nullptr) return;

    // Retrieve the instance of the graphics engine
    graphicsEngine.setVertexArrayObject(meshVBO);

    if (m_mesh->getInstanceCount() > 0)
    {
        // mesh has instances so we render instanced
        graphicsEngine.drawIndexedTrianglesInstanced(TriangleType::TriangleList, meshVBO->getNumIndices(), m_mesh->getInstanceCount());
    }
    else
    {
        graphicsEngine.drawIndexedTriangles(TriangleType::TriangleList, meshVBO->getNumIndices());
    }

    m_shader->setTexture2D(nullptr, 0, "Texture0");
    graphicsEngine.setTextureCubeMap(nullptr, 6, "Texture_Skybox");
    graphicsEngine.setTexture2D(nullptr, 1, "ReflectionMap");
}

void MeshRenderer::SetMesh(MeshPtr mesh)
{
    m_mesh = mesh;
}

void MeshRenderer::SetShadowShader(ShaderPtr shader)
{
    m_shadowShader = shader;
}

void MeshRenderer::SetShader(ShaderPtr shader)
{
    m_shader = shader;
}

void MeshRenderer::SetGeometryShader(ShaderPtr shader)
{
    m_geometryShader = shader;
}

void MeshRenderer::SetTexture(Texture2DPtr texture)
{
    m_texture = texture;
}

void MeshRenderer::SetReflectiveMapTexture(Texture2DPtr texture)
{
    m_reflectiveMap = texture;
}

void MeshRenderer::SetShininess(float shininess)
{
    m_shininess = shininess;
}

void MeshRenderer::SetAlpha(float alpha)
{
    m_alpha = alpha;
}

float MeshRenderer::GetAlpha()
{
    return m_alpha;
}

void MeshRenderer::SetColor(Vector3 color)
{
    m_color = color;
}
