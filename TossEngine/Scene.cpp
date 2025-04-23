/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2024 Media Design School
File Name : Scene.h
Description : Base class representing a game scene, providing essential methods for rendering and updating the game logic.
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
**/

#include "Scene.h"
#include "GameObjectManager.h"
#include "GraphicsEngine.h"
#include "GeometryBuffer.h"
#include "AudioEngine.h"
#include "ComponentRegistry.h"
#include "MeshRenderer.h"
#include "Rigidbody.h"
#include "Image.h"
#include "Material.h"
#include "Ship.h"
#include "PointLight.h"
#include "TossEngine.h"
#include "Skybox.h"
#include "Camera.h"
#include "Framebuffer.h"
#include "Physics.h"
#include "LightManager.h"
#include "DirectionalLight.h"
#include "ImGuizmo.h"

Scene::Scene(const string& filePath)
{
    m_filePath = filePath;

    auto& tossEngine = TossEngine::GetInstance();

    m_lightManager = std::make_unique<LightManager>();
    m_postProcessingFramebuffer = std::make_shared<Framebuffer>(tossEngine.GetWindow()->getInnerSize());
    m_gameObjectManager = std::make_unique<GameObjectManager>(this);

    m_SSRQ = std::make_unique<Image>();
    m_SSRQ->SetSize({ -2.0f, 2.0f });
}

Scene::Scene(const Scene& other)
{
    auto& tossEngine = TossEngine::GetInstance();


    m_lightManager = std::make_unique<LightManager>();
    m_postProcessingFramebuffer = std::make_shared<Framebuffer>(tossEngine.GetWindow()->getInnerSize());

    // Copy GameObjectManager (requires a proper copy constructor or Clone() function)

    m_gameObjectManager = std::make_unique<GameObjectManager>(this);
    //m_gameObjectManager = std::make_unique<GameObjectManager>(*other.m_gameObjectManager);
    m_SSRQ = std::make_unique<Image>();
    m_SSRQ->SetSize({ -2.0f, 2.0f });
}

Scene::~Scene()
{
}

void Scene::clean()
{
    m_gameObjectManager->clearGameObjects();
    m_lightManager->clearLights();
}

void Scene::reload()
{
    onCreate();
    onCreateLate();
}

void Scene::onCreate()
{
    auto& graphicsEngine = GraphicsEngine::GetInstance();
    Physics::GetInstance().LoadWorld();

    auto& resourceManager = ResourceManager::GetInstance();

    //hard coded for now as i need to internalize some shaders and materials so they dont go corrupt by user
    m_deferredSSRQMaterial = resourceManager.createMaterial(MaterialDesc{ "SSQLightingShader" }, "DeferredSSRQMaterial");
    m_postProcessSSRQMaterial = resourceManager.createMaterial(MaterialDesc{ "DefaultFullscreenShader" }, "PostProcessSSRQMaterial");

    json sceneJson = JsonUtility::OpenJsonFile(m_filePath, true);
    if (!sceneJson.empty())
    {
        m_gameObjectManager->loadGameObjectsFromFile(m_filePath);
    }
    else
    {
        Debug::Log("Created new scene at filepath: " + m_filePath);

        //Creates default objects that are usually required for a scene
        GameObjectPtr skyboxObject = m_gameObjectManager->createGameObject<GameObject>("Skybox");
        Skybox* skybox = skyboxObject->addComponent<Skybox>();
        skybox->setMesh(resourceManager.getMesh("Resources/Meshes/cube.obj"));

        GameObjectPtr directionalLightObject = m_gameObjectManager->createGameObject<GameObject>("Directional Light");
        directionalLightObject->m_transform.rotation = Quaternion::FromEuler(Vector3(30,40,50));
        DirectionalLight* dirLight = directionalLightObject->addComponent<DirectionalLight>();


    }
}

void Scene::onStart()
{
    m_gameObjectManager->onStart();
}

void Scene::onLateStart()
{
    m_gameObjectManager->onLateStart();
}

void Scene::onCreateLate()
{
    auto& tossEngine = TossEngine::GetInstance();
    for (auto& camera : m_gameObjectManager->getCameras())
    {
        // Set the screen area for all cameras
        camera->setScreenArea(tossEngine.GetWindow()->getInnerSize());
    }
}

void Scene::onUpdate()
{
    m_gameObjectManager->onUpdate();
}

void Scene::onUpdateInternal()
{
    m_gameObjectManager->onUpdateInternal();
}

void Scene::onFixedUpdate()
{
    m_gameObjectManager->onFixedUpdate();
}

void Scene::onLateUpdate()
{
    m_gameObjectManager->onLateUpdate();
}

void Scene::onGraphicsUpdate(Camera* cameraToRenderOverride, FramebufferPtr writeToFrameBuffer)
{
    auto& tossEngine = TossEngine::GetInstance();
    auto& graphicsEngine = GraphicsEngine::GetInstance();
    graphicsEngine.clear(glm::vec4(0, 0, 0, 1));

    // Populate the uniform data struct
    uniformData.currentTime = tossEngine.GetTime();

    if (cameraToRenderOverride != nullptr)
    {
        cameraToRenderOverride->getViewMatrix(uniformData.viewMatrix);
        cameraToRenderOverride->getProjectionMatrix(uniformData.projectionMatrix);
        uniformData.cameraPosition = cameraToRenderOverride->getPosition();
        m_lightManager->setSpotlightPosition(uniformData.cameraPosition);
        m_lightManager->setSpotlightDirection(cameraToRenderOverride->getFacingDirection());
    }
    else
    {
        // get the camera data from a camera in scene
        for (auto& camera : m_gameObjectManager->getCameras())
        {
            if (camera->getCameraType() == CameraType::Perspective)
            {
                camera->getViewMatrix(uniformData.viewMatrix);
                camera->getProjectionMatrix(uniformData.projectionMatrix);
                uniformData.cameraPosition = camera->getPosition();
                m_lightManager->setSpotlightPosition(uniformData.cameraPosition);
                m_lightManager->setSpotlightDirection(camera->getFacingDirection());
            }
            else
            {
                camera->getViewMatrix(uniformData.uiViewMatrix);
                camera->getProjectionMatrix(uniformData.uiProjectionMatrix);
            }
        }
    }

    // Example of Defered Rendering Pipeline
    if (graphicsEngine.getRenderingPath() == RenderingPath::Deferred)
    {
        //Geometry Pass
        auto& geometryBuffer = GeometryBuffer::GetInstance();
        geometryBuffer.Bind();
        m_gameObjectManager->Render(uniformData);
        geometryBuffer.UnBind();


        // Shadow Pass: Render shadows for directional lights
        for (uint i = 0; i < m_lightManager->getDirectionalLightCount(); i++)
        {
            m_lightManager->BindShadowMap(i);
            m_gameObjectManager->onShadowPass(i); // Render shadow maps
            m_lightManager->UnBindShadowMap(i);
        }
        graphicsEngine.setViewport(tossEngine.GetWindow()->getInnerSize());

        // Lighting Pass: Apply lighting using G-buffer data
        // Populate the shader with geometry buffer information for the lighting pass
        ShaderPtr shader = m_deferredSSRQMaterial->GetShader();
        graphicsEngine.setShader(shader);
        GeometryBuffer::GetInstance().PopulateShader(shader);

        // Apply lighting settings using the LightManager
        m_lightManager->applyLighting(shader);

        // Apply shadows
        m_lightManager->applyShadows(shader);

        m_postProcessingFramebuffer->Bind();

        // Render the screenspace quad using the lighting, shadow and geometry data
        m_SSRQ->SetMaterial(m_deferredSSRQMaterial);
        m_SSRQ->Render(uniformData, RenderingPath::Forward);

        geometryBuffer.WriteDepth(m_postProcessingFramebuffer->getId());

        // Render the transparent objects after
        m_gameObjectManager->onTransparencyPass(uniformData);
        m_gameObjectManager->onSkyboxPass(uniformData);


        if (cameraToRenderOverride)
        {
            Physics::GetInstance().DrawDebug(uniformData);
        }

        m_postProcessingFramebuffer->UnBind();
    }

    // Example of Forward Rendering Pipeline
    if (graphicsEngine.getRenderingPath() == RenderingPath::Forward)
    {
        for (uint i = 0; i < m_lightManager->getDirectionalLightCount(); i++)
        {
            //Shadow Pass
            m_lightManager->BindShadowMap(i);
            m_gameObjectManager->onShadowPass(i);
            m_lightManager->UnBindShadowMap(i);
        }
        graphicsEngine.setViewport(tossEngine.GetWindow()->getInnerSize());

        m_postProcessingFramebuffer->Bind();

        m_gameObjectManager->Render(uniformData);

        // Render the transparent objects after
        m_gameObjectManager->onTransparencyPass(uniformData);
        m_gameObjectManager->onSkyboxPass(uniformData);

        if (cameraToRenderOverride)
        {
            Physics::GetInstance().DrawDebug(uniformData);
        }

        m_postProcessingFramebuffer->UnBind();
    }


    graphicsEngine.clear(glm::vec4(0, 0, 0, 1)); //clear the scene

    if (writeToFrameBuffer != nullptr)
    {
        writeToFrameBuffer->Bind();
    }

    // Post processing 
    m_postProcessingFramebuffer->PopulateShader(m_postProcessSSRQMaterial->GetShader());
    m_SSRQ->SetMaterial(m_postProcessSSRQMaterial);
    m_SSRQ->Render(uniformData, RenderingPath::Forward);

    if (writeToFrameBuffer != nullptr)
    {
        writeToFrameBuffer->UnBind();
    }

    // ui canvas of some sort with the ui camera
}

void Scene::onResize(Vector2 size)
{
    Resizable::onResize(size);
    GeometryBuffer::GetInstance().Resize(size);

    for (auto camera : m_gameObjectManager->getCameras())
    {
        camera->setScreenArea(size);
    }
    // resize the post processing frame buffer 
    m_postProcessingFramebuffer->onResize(size);
}

LightManager* Scene::getLightManager()
{
    return m_lightManager.get();
}

GameObjectManager* Scene::getObjectManager()
{
    return m_gameObjectManager.get();
}

void Scene::onQuit()
{
    m_gameObjectManager->clearGameObjects();
    Physics::GetInstance().UnLoadWorld();
}

void Scene::Save()
{
    json sceneJson = m_gameObjectManager->saveGameObjects();
    if (JsonUtility::SaveJsonFile(m_filePath, sceneJson, true))
    {
        Debug::Log("Scene saved to file path: " + m_filePath);
    }
    else
    {
        Debug::LogError("Could not save the scene to filepath: " + m_filePath);
    }
}

string Scene::GetFilePath()
{
    return m_filePath;
}