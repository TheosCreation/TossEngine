#include "MeshRenderer.h"
#include "GraphicsEngine.h"
#include "LightManager.h"
#include "Mesh.h"
#include "Shader.h"
#include "Entity.h"
#include "Texture.h"
#include "VertexArrayObject.h"

MeshRenderer::MeshRenderer()
{
}

MeshRenderer::~MeshRenderer()
{
}

void MeshRenderer::onShadowPass(uint index)
{
    if (m_shadowShader == nullptr) return;

    auto& graphicsEngine = GraphicsEngine::GetInstance();
    graphicsEngine.setFaceCulling(CullType::BackFace);
    graphicsEngine.setWindingOrder(WindingOrder::CounterClockWise);
    graphicsEngine.setDepthFunc(DepthType::Less);
    graphicsEngine.setShader(m_shadowShader);

    auto& lightManager = LightManager::GetInstance();
    m_shadowShader->setMat4("VPLight", lightManager.getLightSpaceMatrix(index));
    m_shadowShader->setMat4("modelMatrix", m_gameObjectOwnerRef->m_transform.GetMatrix());

    if (m_mesh == nullptr) return;

    // Bind the vertex array object for the mesh
    auto meshVBO = m_mesh->getVertexArrayObject();

    graphicsEngine.setVertexArrayObject(meshVBO);

    // Draw the mesh to update the shadow map
    graphicsEngine.drawIndexedTriangles(TriangleType::TriangleList, meshVBO->getNumIndices());
}

void MeshRenderer::Render(UniformData data)
{
    auto& graphicsEngine = GraphicsEngine::GetInstance();
    graphicsEngine.setFaceCulling(CullType::BackFace);
    graphicsEngine.setWindingOrder(WindingOrder::CounterClockWise);
    graphicsEngine.setDepthFunc(DepthType::Less);

    if (graphicsEngine.getRenderingPath() == RenderingPath::Deferred)
    {
        graphicsEngine.setShader(m_geometryShader);
        m_geometryShader->setMat4("VPMatrix", data.projectionMatrix * data.viewMatrix);
        m_geometryShader->setMat4("modelMatrix", m_gameObjectOwnerRef->m_transform.GetMatrix());
        m_geometryShader->setFloat("ObjectShininess", m_shininess);

        if (m_texture != nullptr)
        {
            graphicsEngine.setTexture2D(m_texture, 0, "Texture0");
        }

        if (m_reflectiveMap)
        {
            graphicsEngine.setTexture2D(m_reflectiveMap, 2, "ReflectionMap");
        }

    }
    

    if (graphicsEngine.getRenderingPath() == RenderingPath::Forward)
    {
        graphicsEngine.setShader(m_shader);
        m_shader->setMat4("VPMatrix", data.projectionMatrix * data.viewMatrix);
        m_shader->setMat4("modelMatrix", m_gameObjectOwnerRef->m_transform.GetMatrix());
        m_shader->setVec3("CameraPos", data.cameraPosition);
        m_shader->setFloat("ObjectShininess", m_shininess);

        LightManager::GetInstance().applyLighting(m_shader);


        auto& graphicsEngine = GraphicsEngine::GetInstance();
        if (m_texture != nullptr)
        {
            graphicsEngine.setTexture2D(m_texture, 0, "Texture0");
        }
        else
        {
            m_shader->setVec3("uColor", m_color);
            m_shader->setFloat("alpha", m_alpha);
        }

        auto skyboxTexture = ResourceManager::GetInstance().getSkyboxTexture();
        if (skyboxTexture)
        {
            graphicsEngine.setTextureCubeMap(skyboxTexture, 1, "Texture_Skybox");
        }

        if (m_reflectiveMap)
        {
            graphicsEngine.setTexture2D(m_reflectiveMap, 2, "ReflectionMap");
        }

        auto& lightManager = LightManager::GetInstance();
        lightManager.applyShadows(m_shader);
    }

    // Bind the vertex array object for the mesh
    auto meshVBO = m_mesh->getVertexArrayObject();

    // Retrieve the instance of the graphics engine
    graphicsEngine.setVertexArrayObject(meshVBO);

    // Draw the mesh to update the shadow map
    graphicsEngine.drawIndexedTriangles(TriangleType::TriangleList, meshVBO->getNumIndices());

    graphicsEngine.setTexture2D(nullptr, 0, "");
    graphicsEngine.setTextureCubeMap(nullptr, 1, "");
    graphicsEngine.setTexture2D(nullptr, 2, "");
    graphicsEngine.setTexture2D(nullptr, 3, "");
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

void MeshRenderer::SetTexture(TexturePtr texture)
{
    m_texture = texture;
}

void MeshRenderer::SetReflectiveMapTexture(TexturePtr texture)
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
