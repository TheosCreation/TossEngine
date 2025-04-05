/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2024 Media Design School
File Name : GameObjectManager.cpp
Description : GameObject system is a container and controller of the entities for the game
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
**/

#include "GameObjectManager.h"
#include "Camera.h"
#include "GraphicsEngine.h"
#include "MeshRenderer.h"
#include "RigidBody.h"
#include "Image.h"
#include "Skybox.h"
#include "TextureCubeMap.h"

GameObjectManager::GameObjectManager()
{
}

GameObjectManager::GameObjectManager(Scene* scene)
{
    m_scene = scene;
}

GameObjectManager::GameObjectManager(const GameObjectManager& other)
{
    m_scene = other.m_scene; // Assuming the scene should be shared

    for (const auto& pair : other.m_gameObjects)
    {
        //pair.second
    }

    // Copy the entities set (no deep copy needed since these objects are managed in m_gameObjects)
    m_gameObjectsToDestroy = other.m_gameObjectsToDestroy;
}

GameObjectManager::~GameObjectManager()
{
}

Scene* GameObjectManager::getScene()
{
    return m_scene;
}

bool GameObjectManager::createGameObjectInternal(GameObject* gameObject, string name)
{
    if (!gameObject)
        return false;

    name = getGameObjectNameAvaliable(name);

    size_t newId = m_nextAvailableId++;
    gameObject->name = name;
    gameObject->setId(newId);
    gameObject->setGameObjectManager(this);
    gameObject->onCreate();
    gameObject->onLateCreate();

    m_gameObjects[newId] = std::move(gameObject);

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
        GameObject* gameObject = pair.second;
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

void GameObjectManager::removeGameObject(GameObject* gameObject)
{
    m_gameObjectsToDestroy.push_back(gameObject->getId());
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
        auto gameObject = new GameObject();
        // Initialize the GameObject
        gameObject->setGameObjectManager(this);
        gameObject->onCreate();
        gameObject->deserialize(gameObjectData);  // Loads data into the object
        gameObject->onLateCreate();

        size_t id = gameObject->getId();
        if (id == 0) id = m_nextAvailableId++;

        gameObject->setId(id);
        m_gameObjects[id] = std::move(gameObject);
    }
}

json GameObjectManager::saveGameObjects()
{
    json data;
    data["gameobjects"] = json::array();

    for (const auto& pair : m_gameObjects)
    {
        data["gameobjects"].push_back(pair.second->serialize());
    }

    return data;
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

    for (const auto& gameObjectData : sceneData["gameobjects"])
    {
        auto gameObject = new GameObject();
        // Initialize the GameObject
        gameObject->setGameObjectManager(this);
        gameObject->onCreate();
        gameObject->deserialize(gameObjectData);  // Loads data into the object
        gameObject->onLateCreate();

        size_t id = gameObject->getId();
        if (id == 0) id = m_nextAvailableId++;

        gameObject->setId(id);
        m_gameObjects[id] = std::move(gameObject);
    }
}

void GameObjectManager::saveGameObjectsToFile(const std::string& filePath)
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

void GameObjectManager::onStart()
{
    for (const auto& pair : m_gameObjects)
    {
        pair.second->onStart();
    }
}

void GameObjectManager::onLateStart()
{
    for (const auto& pair : m_gameObjects)
    {
        pair.second->onLateStart();
    }
}


void GameObjectManager::onUpdate(float deltaTime)
{
    onUpdateInternal();

    for (const auto& pair : m_gameObjects)
    {
        pair.second->onUpdate(deltaTime);
    }
}

void GameObjectManager::onUpdateInternal()
{
    for (size_t gameObjectId : m_gameObjectsToDestroy)
    {
        m_gameObjects.erase(gameObjectId);  // Directly erase by ID
    }
    m_gameObjectsToDestroy.clear();

    for (const auto& pair : m_gameObjects)
    {
        pair.second->onUpdateInternal();
    }
}

void GameObjectManager::onLateUpdate(float deltaTime)
{
    for (const auto& pair : m_gameObjects)
    {
        pair.second->onLateUpdate(deltaTime);
    }
}

void GameObjectManager::onShadowPass(int index)
{
    for (const auto& pair : m_gameObjects)
    {
        MeshRenderer* renderer = pair.second->getComponent<MeshRenderer>();
        if (renderer)
        {
            if (renderer->GetAlpha() < 1.0f) continue; // if the renderer is transparent we skip it

            renderer->onShadowPass(index);
        }
    }
}

void GameObjectManager::Render(UniformData _data)
{
    auto& graphicsEngine = GraphicsEngine::GetInstance();
    for (const auto& pair : m_gameObjects)
    {
        if (auto meshRenderer = pair.second->getComponent<MeshRenderer>())
        {
            if (meshRenderer->GetAlpha() != 1.0f) continue;

            meshRenderer->Render(_data, graphicsEngine.getRenderingPath());
        }
    }
}

void GameObjectManager::onTransparencyPass(UniformData _data)
{
    for (const auto& pair : m_gameObjects)
    {
        if (auto meshRenderer = pair.second->getComponent<MeshRenderer>())
        {
            if (meshRenderer->GetAlpha() == 1.0f) continue;

            meshRenderer->Render(_data, RenderingPath::Forward); // we render the transparent renderers last with forward rendering
        }
    }
}

void GameObjectManager::onSkyboxPass(UniformData _data)
{
    for (const auto& pair : m_gameObjects)
    {
        if (auto skybox = pair.second->getComponent<Skybox>())
        {
            skybox->Render(_data, RenderingPath::Forward);
        }
    }
}

void GameObjectManager::onFixedUpdate(float fixedDeltaTime)
{
    for (const auto& pair : m_gameObjects)
    {
        pair.second->onFixedUpdate(fixedDeltaTime);
    }
}

std::vector<Camera*> GameObjectManager::getCameras() const
{
    std::vector<Camera*> cameras;

    // Iterate over all game objects and check if they have a Camera component
    for (const auto& pair : m_gameObjects)
    {
        Camera* camera = pair.second->getComponent<Camera>();
        if (camera) {
            cameras.push_back(camera);
        }
    }

    return cameras;
}

TexturePtr GameObjectManager::getSkyBoxTexture()
{
    for (const auto& pair : m_gameObjects)
    {
        if (Skybox* skybox = pair.second->getComponent<Skybox>())
        {
            return skybox->GetMaterial()->GetBinding("Texture_Skybox");
        }
    }
}

void GameObjectManager::clearGameObjects()
{
    // Clear the entities in the map
    m_gameObjects.clear();

    m_gameObjectsToDestroy.clear();
}
