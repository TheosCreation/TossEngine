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
#include "GameObject.h"
#include "Camera.h"
#include "GraphicsGameObject.h"
#include "GraphicsEngine.h"
#include "MeshRenderer.h"
#include "RigidBody.h"
#include "Image.h"

GameObjectManager::GameObjectManager()
{
}

GameObjectManager::GameObjectManager(Scene* scene)
{
	m_scene = scene;
}

GameObjectManager::~GameObjectManager()
{
}

Scene* GameObjectManager::getScene()
{
	return m_scene;
}

bool GameObjectManager::createGameObjectInternal(GameObject* gameObject)
{
	auto ptr = std::unique_ptr<GameObject>(gameObject);

	// Check if the GameObject is of type GraphicsGameObject
	if (dynamic_cast<GraphicsGameObject*>(gameObject)) {
		m_graphicsObjects.push_back(static_cast<GraphicsGameObject*>(gameObject));
	}

	ptr->setGameObjectManager(this);
	ptr->onCreate();

	// Add the GameObject to the internal map
	m_gameObjects.push_back(std::move(ptr));

	return true;
}

void GameObjectManager::removeGameObject(GameObject* GameObject)
{
	m_entitiesToDestroy.emplace(GameObject);
}

void GameObjectManager::loadGameObjectsFromFile(const std::string& filePath)
{
	std::ifstream file(filePath);
	if (!file.is_open())
	{
		std::cerr << "Failed to open file: " << filePath << std::endl;
		return;
	}

	nlohmann::json sceneData;
	try
	{
		file >> sceneData;
	}
	catch (const std::exception& e)
	{
		std::cerr << "Failed to parse JSON file: " << e.what() << std::endl;
		return;
	}

	if (!sceneData.contains("gameobjects") || !sceneData["gameobjects"].is_array())
	{
		std::cerr << "Error: JSON does not contain a valid 'gameobjects' array!" << std::endl;
		return;
	}

	for (const auto& gameObjectData : sceneData["gameobjects"])
	{
		auto gameObject = std::make_unique<GameObject>();
		// Initialize the GameObject
		gameObject->setId(0);
		gameObject->setGameObjectManager(this);
		gameObject->onCreate();
		gameObject->deserialize(gameObjectData);  // Loads data into the object

		m_gameObjects.push_back(std::move(gameObject));
	}
}

void GameObjectManager::saveGameObjectsToFile(const std::string& filePath)
{
	json sceneData;
	sceneData["gameobjects"] = json::array();

	for (auto& gameObject : m_gameObjects)
	{
		if (gameObject->getComponent<MeshRenderer>())
		{
			sceneData["gameobjects"].push_back(gameObject->serialize());
		}
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


void GameObjectManager::onUpdate(float deltaTime)
{
	//for (auto e : m_entitiesToDestroy)
	//{
	//	m_gameObjects[e->getId()].erase(e);
	//}
	m_entitiesToDestroy.clear();


	for (auto& gameObject : m_gameObjects)
	{
		gameObject->onUpdate(deltaTime);
	}
}

void GameObjectManager::onLateUpdate(float deltaTime)
{

	for (auto& gameObject : m_gameObjects)
	{
		gameObject->onLateUpdate(deltaTime);
	}
}

void GameObjectManager::onShadowPass(int index)
{
	//for (auto& graphicsGameObject : m_graphicsEntities)
	//{
	//	graphicsGameObject->onShadowPass(index);
	//}

	for (auto& gameObject : m_gameObjects)
	{
		MeshRenderer* renderer = gameObject->getComponent<MeshRenderer>();
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
	for (auto& gameObject : m_gameObjects)
	{
		MeshRenderer* renderer = gameObject->getComponent<MeshRenderer>();
		if (renderer)
		{
			if (renderer->GetAlpha() != 1.0f) continue;

			renderer->Render(_data, graphicsEngine.getRenderingPath());
		}
	}
}

void GameObjectManager::onGeometryPass(UniformData _data)
{
	for (auto& graphicsGameObject : m_graphicsObjects)
	{
		if (graphicsGameObject->getTransparency() < 1.0f) continue; // if the renderer is transparent we skip it

		graphicsGameObject->onGeometryPass(_data);
	}
}

void GameObjectManager::onTransparencyPass(UniformData _data)
{
	for (auto& graphicsGameObject : m_graphicsObjects)
	{
		if (graphicsGameObject->getTransparency() == 1.0f) continue; // if the renderer is opaque we skip it

		graphicsGameObject->onGraphicsUpdate(_data);
	}

	for (auto& gameObject : m_gameObjects)
	{
		MeshRenderer* renderer = gameObject->getComponent<MeshRenderer>();
		if (renderer)
		{
			if(renderer->GetAlpha() == 1.0f) continue;

			renderer->Render(_data, RenderingPath::Forward); // we render the transparent renderers last with forward rendering
		}

	}
}

void GameObjectManager::onGraphicsUpdate(UniformData _data)
{
	for (auto& graphicsGameObject : m_graphicsObjects)
	{
		// Apply other uniform data to the shader
		graphicsGameObject->onGraphicsUpdate(_data);
	}
}

void GameObjectManager::onFixedUpdate(float fixedDeltaTime)
{
	for (auto& gameObject : m_gameObjects)
	{
		gameObject->onFixedUpdate(fixedDeltaTime); 
	}
}

std::vector<GraphicsGameObject*> GameObjectManager::getGraphicsEntities() const
{
	return m_graphicsObjects;
}

std::vector<Camera*> GameObjectManager::getCameras() const
{
	std::vector<Camera*> cameras;

	// Iterate over all game objects and check if they have a Camera component
	for (auto& gameObject : m_gameObjects)
	{
		Camera* camera = gameObject->getComponent<Camera>();
		if (camera) {
			cameras.push_back(camera);
		}
	}

	return cameras;
}

void GameObjectManager::clearGameObjects()
{
	// Clear the entities in the map
	m_gameObjects.clear();

	// Clear the graphics entities vector
	m_graphicsObjects.clear();

	// Clear the cameras vector
	m_cameras.clear();

	m_entitiesToDestroy.clear();
}
