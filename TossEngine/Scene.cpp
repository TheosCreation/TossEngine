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
#include "GraphicsEngine.h"
#include "GeometryBuffer.h"
#include "AudioEngine.h"
#include "MeshRenderer.h"
#include "Rigidbody.h"
#include "Image.h"
#include "Material.h"
#include "TossEngine.h"
#include "Skybox.h"
#include "Camera.h"
#include "Framebuffer.h"
#include "Physics.h"
#include "LightManager.h"
#include "DirectionalLight.h"
#include "Renderer.h"
#include "Text.h"
#include "Prefab.h"

Scene::Scene(const string& filePath)
{
    m_filePath = filePath;

    auto& tossEngine = TossEngine::GetInstance();

    m_lightManager = std::make_unique<LightManager>();
    m_postProcessingFramebuffer = std::make_shared<Framebuffer>(tossEngine.GetWindow()->getInnerSize());

    m_SSRQ = std::make_unique<Image>();
    m_SSRQ->SetSize({ -2.0f, 2.0f });
}

Scene::Scene(const Scene& other)
{
    auto& tossEngine = TossEngine::GetInstance();


    m_lightManager = std::make_unique<LightManager>();
    m_postProcessingFramebuffer = std::make_shared<Framebuffer>(tossEngine.GetWindow()->getInnerSize());

    // Copy Scene (requires a proper copy constructor or Clone() function)

    //m_scene = std::make_unique<Scene>(*other.m_scene);
    m_SSRQ = std::make_unique<Image>();
    m_SSRQ->SetSize({ -2.0f, 2.0f });
}

Scene::~Scene()
{
}

void Scene::clean()
{
    clearGameObjects();
    m_lightManager->clearLights();
}

void Scene::reload()
{
    onCreate();
}

GameObjectPtr Scene::Instantiate(const PrefabPtr& prefab, Transform* parent, Vector3 positionalOffset,
    Quaternion rotationOffset, bool hasStarted)
{
    GameObjectPtr newObject = prefab->Instantiate();
    if (!newObject)
    {
        Debug::LogError("Failed to instantiate prefab.", false);
        return nullptr;
    }

    // Set parent if provided (update local transform accordingly)
    if (parent != nullptr)
    {
        newObject->m_transform.SetParent(parent, false);
        newObject->m_transform.localPosition = positionalOffset;
        newObject->m_transform.localRotation = rotationOffset;
    }
    else
    {

        newObject->m_transform.position = positionalOffset;
        newObject->m_transform.rotation = rotationOffset;
    }

    if (createGameObjectInternal(newObject, prefab->name))
    {
        if (hasStarted)
        {
            newObject->onStart();
            newObject->onLateStart();
        }
        return newObject;
    }

    return nullptr;
}

GameObjectPtr Scene::Instantiate(const PrefabPtr& prefab, Vector3 position, Quaternion rotation, bool hasStarted)
{
    return Instantiate(prefab, nullptr, position, rotation, hasStarted);
}

void Scene::deleteGameObject(const GameObject* gameObject, float _delay)
{
    if (gameObject && !gameObject->isDestroyed && !m_objectsToDestroy.contains(gameObject->getId()))
    {
        m_objectsToDestroy[gameObject->getId()] = _delay;
    }
}

void Scene::loadGameObjects(const json& data)
{
    if (!data.contains("gameobjects") || !data["gameobjects"].is_array())
    {
        Debug::LogError("Error: JSON does not contain a valid 'gameobjects' array!", false);
        return;
    }

    for (const auto& gameObjectData : data["gameobjects"])
    {
        auto gameObject = std::make_shared<GameObject>();
        // Initialize the GameObject
        gameObject->setScene(this);
        gameObject->deserialize(gameObjectData);  // Loads data into the object
        if (m_initilized)
        {
            gameObject->onCreate();
            gameObject->onCreateLate();
        }

        size_t id = gameObject->getId();
        if (id == 0) id = m_nextAvailableId++;

        gameObject->setId(id);
        m_gameObjects[id] = std::move(gameObject);
    }
}

json Scene::saveGameObjects() const
{
    json data;
    data["gameobjects"] = json::array();

    for (const auto& pair : m_gameObjects)
    {
        data["gameobjects"].push_back(pair.second->serialize());
    }

    return data;
}

void Scene::loadGameObjectsFromFile(const std::string& filePath)
{
    std::ifstream file(filePath);
    if (!file.is_open())
    {
        Debug::LogError("Failed to open scene file at: " + filePath + ". Please open a new scene file", false);
        return;
    }

    json sceneData;
    try
    {
        file >> sceneData;
    }
    catch (const std::exception& e)
    {
        Debug::LogError("Failed to parse JSON file: " + (string)e.what(), false);
        return;
    }

    if (!sceneData.contains("gameobjects") || !sceneData["gameobjects"].is_array())
    {
        Debug::LogError("Error: JSON does not contain a valid 'gameobjects' array!", false);
        return;
    }

    std::set<size_t> usedIds;
    std::vector<size_t>  idOrder;
    idOrder.reserve(sceneData["gameobjects"].size());

    for (auto& gameObjectData : sceneData["gameobjects"])
    {
        // read saved ID (or 0 if missing)
        size_t savedId = 0;
        if (gameObjectData.contains("id"))
            savedId = gameObjectData["id"].get<size_t>();

        // if that ID is already taken, bump to nextAvailable
        if (savedId == 0 || usedIds.count(savedId))
            savedId = m_nextAvailableId++;

        // reserve that ID
        usedIds.insert(savedId);

        idOrder.push_back(savedId);

        // instantiate
        auto gameObject = std::make_shared<GameObject>();
        gameObject->setScene(this);
        gameObject->setId(savedId);

        // put in map
        m_gameObjects[savedId] = gameObject;
    }

    // second pass for deserilization and create functions
    m_nextAvailableId = std::max(m_nextAvailableId,
        *std::max_element(usedIds.begin(), usedIds.end()) + 1
    );
    for (size_t i = 0; i < idOrder.size(); ++i)
    {
        size_t finalId = idOrder[i];
        auto& gameObject = m_gameObjects[finalId];
        auto& data = sceneData["gameobjects"][i];

        gameObject->deserialize(data);
        if (m_initilized)
        {
            gameObject->onCreate();
            gameObject->onCreateLate();
        }
    }
}

void Scene::saveGameObjectsToFile(const std::string& filePath) const
{
    json sceneData;
    sceneData["gameobjects"] = json::array();

    for (const auto& pair : m_gameObjects)
    {
        sceneData["gameobjects"].push_back(pair.second->serialize());
    }

    std::ofstream file(filePath);
    if (!file.is_open())
    {
        std::cerr << "Failed to open file for writing: " << filePath << std::endl;
        return;
    }

    file << sceneData.dump(4); // Pretty-print JSON with indentation
    file.close();
}

void Scene::onCreate()
{
    auto& graphicsEngine = GraphicsEngine::GetInstance();
    Physics::GetInstance().LoadWorld();

    auto& resourceManager = ResourceManager::GetInstance();

    // TODO: hard coded for now as i need to internalize some shaders and materials so they dont go corrupt by user
    m_deferredSSRQMaterial = resourceManager.get<Material>("DeferredSSRQMaterial");
    m_postProcessSSRQMaterial = resourceManager.get<Material>("PostProcessSSRQMaterial");

    json sceneJson = JsonUtility::OpenJsonFile(m_filePath, true);
    if (!sceneJson.empty())
    {
        loadGameObjectsFromFile(m_filePath);
    }
    else
    {
        Debug::Log("Created new scene at filepath: " + m_filePath);

        //Creates default objects that are usually required for a scene
        GameObjectPtr skyboxObject = createGameObject<GameObject>("Skybox");
        Skybox* skybox = skyboxObject->addComponent<Skybox>();
        skybox->setMesh(resourceManager.get<Mesh>("Resources/Meshes/cube.obj"));

        GameObjectPtr directionalLightObject = createGameObject<GameObject>("Directional Light");
        directionalLightObject->m_transform.rotation = Quaternion::FromEuler(Vector3(30,40,50));
        DirectionalLight* dirLight = directionalLightObject->addComponent<DirectionalLight>();
    }

    for (const auto& pair : m_gameObjects)
    {
        if (!pair.second) continue;

        pair.second->onCreate();
    }

    for (const auto& pair : m_gameObjects)
    {
        if (!pair.second) continue;

        pair.second->onCreateLate();
    }

    auto& tossEngine = TossEngine::GetInstance();
    for (auto& camera : getCameras())
    {
        // Set the screen area for all cameras
        camera->setScreenArea(tossEngine.GetWindow()->getInnerSize());
    }

    m_initilized = true;
}

void Scene::onStart()
{
    if (!m_initilized) return;

    for (const auto& pair : m_gameObjects)
    {
        if (!pair.second) continue; //|| !pair.second->GetActive() and then call on start later

        pair.second->onStart();
    }

    for (const auto& pair : m_gameObjects)
    {
        if (!pair.second || !pair.second->GetActive()) continue;

        pair.second->onLateStart();
    }
}

void Scene::onUpdate()
{
    if (!m_initilized) return;

    if (lastCameraToRender) AudioEngine::GetInstance().set3DListener(lastCameraToRender->getTransform());

    for (const auto& pair : m_gameObjects)
    {
        if (!pair.second || !pair.second->GetActive()) continue;

        pair.second->onUpdate();
    }

    //onUpdateInternal(); //should not need this or else we are updating twice
}

void Scene::onUpdateInternal()
{
    if (!m_initilized) return;

    for (const auto& pair : m_gameObjects)
    {
        if (!pair.second) continue;

        pair.second->onUpdateInternal();
    }
    std::unordered_map<size_t, float> frame;
    frame.swap(m_objectsToDestroy); // m_objectsToDestroy is now empty

    std::unordered_map<size_t, float>::iterator it = frame.begin();

    while (it != frame.end())
    {
        size_t deadId = it->first;
        float remaining = it->second - Time::DeltaTime;

        if (remaining > 0.0f)
        {
            m_objectsToDestroy[deadId] = remaining;
            ++it;
            continue;
        }

        DestroyImmediate(deadId);

        ++it;
    }
}

void Scene::onFixedUpdate()
{
    if (!m_initilized) return;

    for (const auto& pair : m_gameObjects)
    {
        if (!pair.second || !pair.second->GetActive()) continue;

        pair.second->onFixedUpdate();
    }
}

void Scene::onLateUpdate()
{
    if (!m_initilized) return;

    for (const auto& pair : m_gameObjects)
    {
        if (!pair.second || !pair.second->GetActive()) continue;

        pair.second->onLateUpdate();
    }
}

void Scene::onGraphicsUpdate(Camera* cameraToRenderOverride, FramebufferPtr writeToFrameBuffer)
{
    if (!m_initilized) return;

    auto& tossEngine = TossEngine::GetInstance();
    auto& graphicsEngine = GraphicsEngine::GetInstance();
    graphicsEngine.clear(glm::vec4(0, 0, 0, 1));

    // Populate the uniform data struct
    uniformData.currentTime = tossEngine.GetTime();
    if (writeToFrameBuffer)
    {
        uniformData.uiScreenSize = writeToFrameBuffer->getSize();
    }
    else
    {
        uniformData.uiScreenSize = tossEngine.GetWindow()->getInnerSize();
    }

    bool drawUi = true;
    if (cameraToRenderOverride != nullptr)
    {
        drawUi = false;
        cameraToRenderOverride->getViewMatrix(uniformData.viewMatrix);
        cameraToRenderOverride->getProjectionMatrix(uniformData.projectionMatrix);
        uniformData.cameraPosition = cameraToRenderOverride->getPosition();
        m_lightManager->setSpotlightPosition(uniformData.cameraPosition);
        m_lightManager->setSpotlightDirection(cameraToRenderOverride->getFacingDirection());
    }
    else
    {
        vector<Camera*> cameras = getCameras();
        if (cameras.size() > 0)
        {
            lastCameraToRender = cameras[0];
        }
        // get the camera data from a camera in scene
        for (auto& camera : cameras)
        {
            if (camera->getCameraType() == CameraType::Perspective)
            {

                camera->getViewMatrix(uniformData.viewMatrix);
                camera->getProjectionMatrix(uniformData.projectionMatrix);
                uniformData.cameraPosition = camera->getPosition();
                m_lightManager->setSpotlightPosition(uniformData.cameraPosition);
                m_lightManager->setSpotlightDirection(camera->getFacingDirection());
                drawUi = camera->GetDrawUi();
            }
        }
    }
    Window* window = TossEngine::GetInstance().GetWindow();
    Vector2 innerSize = window->getInnerSize();
    if (writeToFrameBuffer)
    {
        innerSize = writeToFrameBuffer->getSize();
    }


    uniformData.uiProjectionMatrix = glm::ortho(0.0f, innerSize.x, -innerSize.y, 0.0f, -1.0f, 1.0f);

    // Example of Defered Rendering Pipeline
    if (graphicsEngine.getRenderingPath() == RenderingPath::Deferred)
    {
        //Geometry Pass
        auto& geometryBuffer = GeometryBuffer::GetInstance();
        geometryBuffer.Bind();

        auto& graphicsEngine = GraphicsEngine::GetInstance();
        for (const auto& pair : m_gameObjects)
        {
            if (!pair.second || !pair.second->GetActive()) continue;

            if (auto meshRenderer = pair.second->getComponent<MeshRenderer>())
            {
                if (meshRenderer->GetAlpha() != 1.0f) continue;

                meshRenderer->Render(uniformData, graphicsEngine.getRenderingPath());
            }
        }

        geometryBuffer.UnBind();


        // Shadow Pass: Render shadows for directional lights
        for (uint i = 0; i < m_lightManager->getDirectionalLightCount(); i++)
        {
            m_lightManager->BindShadowMap(i);
            onShadowPass(i); // Render shadow maps
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
        onTransparencyPass(uniformData);
        onSkyboxPass(uniformData);


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
            onShadowPass(i);
            m_lightManager->UnBindShadowMap(i);
        }
        graphicsEngine.setViewport(tossEngine.GetWindow()->getInnerSize());

        m_postProcessingFramebuffer->Bind();

        auto& graphicsEngine = GraphicsEngine::GetInstance();
        for (const auto& pair : m_gameObjects)
        {
            if (!pair.second || !pair.second->GetActive()) continue;

            if (auto meshRenderer = pair.second->getComponent<MeshRenderer>())
            {
                if (meshRenderer->GetAlpha() != 1.0f) continue;

                meshRenderer->Render(uniformData, graphicsEngine.getRenderingPath());
            }
        }

        // Render the transparent objects after
        onTransparencyPass(uniformData);
        onSkyboxPass(uniformData);

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
    ShaderPtr postProcessShader = m_postProcessSSRQMaterial->GetShader();
    m_postProcessingFramebuffer->PopulateShader(postProcessShader);

    postProcessShader->setFloat("Time", TossEngine::GetTime());
    postProcessShader->setVec2("Resolution", window->getInnerSize());


    m_SSRQ->SetMaterial(m_postProcessSSRQMaterial);
    m_SSRQ->Render(uniformData, RenderingPath::Forward);

    if (drawUi)
    {
        onScreenSpacePass(uniformData);
    }

    if (writeToFrameBuffer != nullptr)
    {
        writeToFrameBuffer->UnBind();
    }

}

void Scene::onShadowPass(int index) const
{
    for (const auto& pair : m_gameObjects)
    {
        if (!pair.second) continue;

        MeshRenderer* renderer = pair.second->getComponent<MeshRenderer>();
        if (renderer)
        {
            if (renderer->GetAlpha() < 1.0f) continue; // if the renderer is transparent we skip it

            renderer->onShadowPass(index);
        }
    }
}

void Scene::onTransparencyPass(UniformData _data) const
{
    for (const auto& pair : m_gameObjects)
    {
        if (!pair.second || !pair.second->GetActive()) continue;

        if (auto meshRenderer = pair.second->getComponent<MeshRenderer>())
        {
            if (meshRenderer->GetAlpha() == 1.0f) continue;

            meshRenderer->Render(_data, RenderingPath::Forward); // we render the transparent renderers last with forward rendering
        }

        if (auto text = pair.second->getComponent<Text>())
        {
            if (text->GetIsUi()) continue;

            text->Render(_data, RenderingPath::Forward);
        }

        if (auto image = pair.second->getComponent<Image>())
        {
            if (image->GetIsUi()) continue;

            image->Render(_data, RenderingPath::Forward);
        }
    }
}

void Scene::onSkyboxPass(UniformData _data) const
{
    for (const auto& pair : m_gameObjects)
    {
        if (!pair.second || !pair.second->GetActive()) continue;

        if (auto skybox = pair.second->getComponent<Skybox>())
        {
            skybox->Render(_data, RenderingPath::Forward);
        }
    }
}

void Scene::onScreenSpacePass(UniformData _data) const
{
    for (const auto& pair : m_gameObjects)
    {
        if (!pair.second || !pair.second->GetActive()) continue;

        if (auto image = pair.second->getComponent<Image>())
        {
            if (!image->GetIsUi()) continue;

            image->Render(_data, RenderingPath::Forward);
        }

        if (auto text = pair.second->getComponent<Text>())
        {
            if (!text->GetIsUi()) continue;

            text->Render(_data, RenderingPath::Forward);
        }
    }
}

void Scene::onResize(Vector2 size)
{
    Resizable::onResize(size);
    GeometryBuffer::GetInstance().Resize(size);

    for (auto camera : getCameras())
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

Window* Scene::getWindow()
{
    return TossEngine::GetInstance().GetWindow();
}

void Scene::SetPostProcessMaterial(MaterialPtr material)
{
    m_postProcessSSRQMaterial = material;
}

void Scene::onQuit()
{

    for (const auto& pair : m_gameObjects)
    {
        if (!pair.second) continue;

        pair.second->onDestroy();
    }
    clearGameObjects();

    m_deferredSSRQMaterial.reset();
    m_postProcessSSRQMaterial.reset();
    m_SSRQ.reset();
    Physics::GetInstance().UnLoadWorld();
    m_initilized = false;
}

GameObjectPtr Scene::getGameObject(size_t id)
{
    return m_gameObjects[id];
}

std::vector<Camera*> Scene::getCameras() const
{
    std::vector<Camera*> cameras;

    // Iterate over all game objects and check if they have a Camera component
    for (const auto& pair : m_gameObjects)
    {
        if (!pair.second) continue;

        if (Camera* camera = pair.second->getComponent<Camera>()) {
            cameras.push_back(camera);
        }
    }

    return cameras;
}

TextureCubeMapPtr Scene::getSkyBoxTexture() const
{
    for (const auto& pair : m_gameObjects)
    {
        if (!pair.second) continue;

        if (Skybox* skybox = pair.second->getComponent<Skybox>())
        {
            return skybox->GetMaterial()->GetBinding("Texture_Skybox");
        }
    }
    return nullptr;
}

void Scene::clearGameObjects()
{
    // Clear the entities in the map
    m_gameObjects.clear();
    m_objectsToDestroy.clear();
}

void Scene::Save()
{
    json sceneJson = saveGameObjects();
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

bool Scene::createGameObjectInternal(GameObjectPtr gameObject, const std::string& name, const json& data)
{
    if (!gameObject)
        return false;


    size_t newId = m_nextAvailableId++;
    gameObject->setId(newId);
    gameObject->setScene(this);
    if (data != nullptr)
    {
        gameObject->deserialize(data);
        string uniqueName = getGameObjectNameAvaliable(gameObject->name);
        gameObject->name = uniqueName;

    }
    else
    {
        string uniqueName = getGameObjectNameAvaliable(name);
        gameObject->name = uniqueName;
    }
    gameObject->onCreate();
    gameObject->onCreateLate();

    m_gameObjects[newId] = gameObject;

    return true;
}

std::string Scene::getGameObjectNameAvaliable(std::string currentName)
{
    // Step 1: Default to "NewGameObject" if name is empty
    if (currentName.empty()) {
        currentName = "NewGameObject";
    }

    // Step 2: Collect all names of root GameObjects
    std::unordered_set<std::string> existingNames;
    for (const auto& pair : m_gameObjects)
    {
        if (!pair.second) continue;

        GameObjectPtr gameObject = pair.second;
        if (gameObject->m_transform.parent == nullptr)
        {
            existingNames.insert(gameObject->name);
        }
    }

    // Step 3: Check if currentName is available
    if (existingNames.find(currentName) == existingNames.end()) {
        return currentName;
    }

    // Step 4: Append (1), (2), ... until we find an available name
    int index = 1;
    std::string candidateName;
    do {
        candidateName = currentName + " (" + std::to_string(index++) + ")";
    } while (existingNames.find(candidateName) != existingNames.end());

    return candidateName;
}

void Scene::DestroyImmediate(size_t _gameobjectId)
{
    auto it = m_gameObjects.find(_gameobjectId);
    if (it == m_gameObjects.end())
        return; // not owned

    GameObject* go = it->second.get();
    if (!go)
        return;

    // prevent double free
    if (go->isDestroyed)
        return;

    go->isDestroyed = true;

    // --- collect children first ---
    std::vector<GameObject*> childObjects;
    auto& kids = go->m_transform.children;
    childObjects.reserve(kids.size());

    for (Transform* ct : kids)
    {
        if (ct && ct->gameObject)
        {
            childObjects.push_back(ct->gameObject);
            ct->parent = nullptr;
        }
    }
    kids.clear();

    // recursively destroy children
    for (GameObject* child : childObjects)
    {
        if (child)
            DestroyImmediate(child->getId());
    }

    // if scheduled to destroy later, remove from pending
    auto pend = m_objectsToDestroy.find(_gameobjectId);
    if (pend != m_objectsToDestroy.end())
        m_objectsToDestroy.erase(pend);

    // call destructor logic
    go->onDestroy();

    // erase from ownership map (deletes unique_ptr)
    m_gameObjects.erase(it);
}
