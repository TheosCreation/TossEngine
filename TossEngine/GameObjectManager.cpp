/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2024 Media Design School
File Name : GameObjectManager.cpp
Description : GameObject system is a container and controller of the game objects for the game
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
**/

#include "GameObjectManager.h"
#include "Camera.h"
#include "GraphicsEngine.h"
#include "Renderer.h"
#include "MeshRenderer.h"
#include "RigidBody.h"
#include "Image.h"
#include "Text.h"
#include "Skybox.h"
#include "TextureCubeMap.h"
#include "Prefab.h"

GameObjectManager::GameObjectManager(Scene* scene)
{
    m_scene = scene;
}

GameObjectManager::GameObjectManager(const GameObjectManager& other)
{
    m_scene = other.m_scene;

    for (const auto& pair : other.m_gameObjects)
    {
    }

    m_gameObjectsToDestroy = other.m_gameObjectsToDestroy;
}

GameObjectPtr GameObjectManager::Instantiate(const PrefabPtr& prefab, Transform* parent, Vector3 positionalOffset, Quaternion rotationOffset, bool hasStarted)
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

        newObject->m_transform.localPosition = positionalOffset;
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

GameObjectPtr GameObjectManager::Instantiate(const PrefabPtr& prefab, Vector3 position, Quaternion rotation, bool hasStarted)
{
    return Instantiate(prefab, nullptr, position, rotation, hasStarted);
}

bool GameObjectManager::createGameObjectInternal(GameObjectPtr gameObject, const string& name, const json& data)
{
    if (!gameObject)
        return false;


    size_t newId = m_nextAvailableId++;
    gameObject->setId(newId);
    gameObject->setGameObjectManager(this);
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


string GameObjectManager::getGameObjectNameAvaliable(string currentName)
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

void GameObjectManager::removeGameObject(const GameObject* gameObject)
{
    if (gameObject && !gameObject->isDestroyed && !m_gameObjectsToDestroy.contains(gameObject->getId()))
    {
        m_gameObjectsToDestroy.insert(gameObject->getId());
    }
}

void GameObjectManager::loadGameObjects(const json& data)
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
        gameObject->setGameObjectManager(this);
        gameObject->deserialize(gameObjectData);  // Loads data into the object
        gameObject->onCreate();
        gameObject->onCreateLate();

        size_t id = gameObject->getId();
        if (id == 0) id = m_nextAvailableId++;

        gameObject->setId(id);
        m_gameObjects[id] = std::move(gameObject);
    }
}

json GameObjectManager::saveGameObjects() const
{
    json data;
    data["gameobjects"] = json::array();

    for (const auto& pair : m_gameObjects)
    {
        data["gameobjects"].push_back(pair.second->serialize());
    }

    return data;
}

GameObjectPtr GameObjectManager::getGameObject(size_t id)
{
    return m_gameObjects[id];
}

void GameObjectManager::loadGameObjectsFromFile(const std::string& filePath)
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
        gameObject->setGameObjectManager(this);
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
        auto& go = m_gameObjects[finalId];
        auto& data = sceneData["gameobjects"][i];

        go->deserialize(data);
        go->onCreate();
        go->onCreateLate(); // maybe split this up to a 3 pass
    }
}

void GameObjectManager::saveGameObjectsToFile(const std::string& filePath) const
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

void GameObjectManager::onStart() const
{
    for (const auto& pair : m_gameObjects)
    {
        if (!pair.second) continue;

        pair.second->onStart();
    }
}

void GameObjectManager::onLateStart() const
{
    for (const auto& pair : m_gameObjects)
    {
        if (!pair.second) continue;

        pair.second->onLateStart();
    }
}


void GameObjectManager::onUpdate()
{
    for (const auto& pair : m_gameObjects)
    {
        if (!pair.second) continue;

        pair.second->onUpdate();
    }

    onUpdateInternal();
}

void GameObjectManager::onUpdateInternal()
{
    for (const auto& pair : m_gameObjects)
    {
        if (!pair.second) continue;

        pair.second->onUpdateInternal();
    }

    for (size_t gameObjectId : m_gameObjectsToDestroy)
    {
        if (auto gameObject = m_gameObjects[gameObjectId])
        {
            gameObject->onDestroy();
            gameObject->isDestroyed = true;
            m_gameObjects.erase(gameObjectId);  // Directly erase by ID
        }
    }
    m_gameObjectsToDestroy.clear();
}

void GameObjectManager::onLateUpdate() const
{
    for (const auto& pair : m_gameObjects)
    {
        if (!pair.second) continue;

        pair.second->onLateUpdate();
    }
}

void GameObjectManager::onShadowPass(int index) const
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

void GameObjectManager::Render(UniformData _data) const
{
    auto& graphicsEngine = GraphicsEngine::GetInstance();
    for (const auto& pair : m_gameObjects)
    {
        if (!pair.second || !pair.second->isActive) continue;

        if (auto meshRenderer = pair.second->getComponent<MeshRenderer>())
        {
            if (meshRenderer->GetAlpha() != 1.0f) continue;

            meshRenderer->Render(_data, graphicsEngine.getRenderingPath());
        }
    }
}

void GameObjectManager::onTransparencyPass(UniformData _data) const
{
    for (const auto& pair : m_gameObjects)
    {
        if (!pair.second || !pair.second->isActive) continue;

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

void GameObjectManager::onSkyboxPass(UniformData _data) const
{
    for (const auto& pair : m_gameObjects)
    {
        if (!pair.second || !pair.second->isActive) continue;

        if (auto skybox = pair.second->getComponent<Skybox>())
        {
            skybox->Render(_data, RenderingPath::Forward);
        }
    }
}

void GameObjectManager::onScreenSpacePass(UniformData _data) const
{
    for (const auto& pair : m_gameObjects)
    {
        if (!pair.second || !pair.second->isActive) continue;

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

void GameObjectManager::onFixedUpdate() const
{
    for (const auto& pair : m_gameObjects)
    {
        if (!pair.second || !pair.second->isActive) continue;

        pair.second->onFixedUpdate();
    }
}

void GameObjectManager::onDestroy() const
{

    for (const auto& pair : m_gameObjects)
    {
        if (!pair.second) continue;

        pair.second->onDestroy();
    }
}

std::vector<Camera*> GameObjectManager::getCameras() const
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

TextureCubeMapPtr GameObjectManager::getSkyBoxTexture() const
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

void GameObjectManager::clearGameObjects()
{
    // Clear the entities in the map
    m_gameObjects.clear();

    m_gameObjectsToDestroy.clear();
}
