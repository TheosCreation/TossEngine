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

GameObjectManager::GameObjectManager()
{
	m_gameObjectFactory = std::make_unique<GameObjectFactory>();
}

GameObjectManager::GameObjectManager(Scene* scene)
{
	m_gameObjectFactory = std::make_unique<GameObjectFactory>();
	m_scene = scene;
}

GameObjectManager::~GameObjectManager()
{
}

Scene* GameObjectManager::getScene()
{
	return m_scene;
}

bool GameObjectManager::createGameObjectInternal(GameObject* gameObject, size_t id)
{
	auto ptr = std::unique_ptr<GameObject>(gameObject);

	// Check if the GameObject is of type GraphicsGameObject
	if (dynamic_cast<GraphicsGameObject*>(gameObject)) {
		m_graphicsEntities.push_back(static_cast<GraphicsGameObject*>(gameObject));
	}

	// Add the GameObject to the internal map
	m_gameObjects[id].emplace(gameObject, std::move(ptr));

	// Initialize the GameObject
	gameObject->setId(id);
	gameObject->setGameObjectManager(this);
	gameObject->onCreate();

	return true;
}

void GameObjectManager::removeGameObject(GameObject* GameObject)
{
	m_entitiesToDestroy.emplace(GameObject);
}

void GameObjectManager::loadEntitiesFromFile(const std::string& filePath)
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

	if (!sceneData.contains("entities"))
	{
		std::cerr << "Error: JSON does not contain 'entities' key!" << std::endl;
		return;
	}

	if (!m_gameObjectFactory)
	{
		std::cerr << "Error: m_GameObjectFactory is null!" << std::endl;
		return;
	}

	for (const auto& GameObjectData : sceneData["entities"])
	{
		if (!GameObjectData.contains("type") || !GameObjectData["type"].is_string())
		{
			std::cerr << "Error: GameObject does not have a valid 'type' field!" << std::endl;
			continue;
		}

		std::string type = GameObjectData["type"];
		std::cout << "Trying to create GameObject of type: " << type << std::endl;

		GameObject* GameObject = m_gameObjectFactory->createGameObject(type);
		if (GameObject)
		{
			std::cout << "GameObject created successfully: " << type << std::endl;
			GameObject->deserialize(GameObjectData);
			std::cout << "GameObject hash code: " << typeid(*GameObject).hash_code() << std::endl;
			createGameObjectInternal(GameObject, typeid(*GameObject).hash_code());
		}
		else
		{
			std::cerr << "Unknown GameObject type: " << type << std::endl;
		}
	}
}


void GameObjectManager::saveEntitiesToFile(const std::string& filePath)
{
	json sceneData;
	sceneData["entities"] = json::array();

	for (auto&& [id, entities] : m_gameObjects)
	{
		for (auto&& [ptr, GameObject] : entities)
		{
			if (GameObject)
			{
				sceneData["entities"].push_back(GameObject->serialize());
			}
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
	for (auto e : m_entitiesToDestroy)
	{
		m_gameObjects[e->getId()].erase(e);
	}
	m_entitiesToDestroy.clear();


	for (auto&& [id, entities] : m_gameObjects)
	{
		for (auto&& [ptr, GameObject] : entities)
		{
			GameObject->onUpdate(deltaTime);
		}
	}
}

void GameObjectManager::onLateUpdate(float deltaTime)
{
	for (auto&& [id, entities] : m_gameObjects)
	{
		for (auto&& [ptr, GameObject] : entities)
		{
			GameObject->onLateUpdate(deltaTime);
		}
	}
}

void GameObjectManager::onShadowPass(int index)
{
	//for (auto& graphicsGameObject : m_graphicsEntities)
	//{
	//	graphicsGameObject->onShadowPass(index);
	//}

	for (auto&& [id, gameObjects] : m_gameObjects)
	{
		for (auto&& [ptr, gameObject] : gameObjects)
		{
			MeshRenderer* renderer = gameObject->getComponent<MeshRenderer>();
			if (renderer)
			{
				if (renderer->GetAlpha() < 1.0f) continue; // if the renderer is transparent we skip it

				renderer->onShadowPass(index);
			}
		}
	}
}

void GameObjectManager::Render(UniformData _data)
{
	auto& graphicsEngine = GraphicsEngine::GetInstance();
	for (auto&& [id, gameObjects] : m_gameObjects)
	{
		for (auto&& [ptr, gameObject] : gameObjects)
		{
			MeshRenderer* renderer = gameObject->getComponent<MeshRenderer>();
			if (renderer)
			{
				if (renderer->GetAlpha() != 1.0f) continue;

				renderer->Render(_data, graphicsEngine.getRenderingPath());
			}
		}
	}
}

void GameObjectManager::onGeometryPass(UniformData _data)
{
	for (auto& graphicsGameObject : m_graphicsEntities)
	{
		if (graphicsGameObject->getTransparency() < 1.0f) continue; // if the renderer is transparent we skip it

		graphicsGameObject->onGeometryPass(_data);
	}
}

void GameObjectManager::onTransparencyPass(UniformData _data)
{
	for (auto& graphicsGameObject : m_graphicsEntities)
	{
		if (graphicsGameObject->getTransparency() == 1.0f) continue; // if the renderer is opaque we skip it

		graphicsGameObject->onGraphicsUpdate(_data);
	}

	for (auto&& [id, gameObjects] : m_gameObjects)
	{
		for (auto&& [ptr, gameObject] : gameObjects)
		{
			MeshRenderer* renderer = gameObject->getComponent<MeshRenderer>();
			if (renderer)
			{
				if(renderer->GetAlpha() == 1.0f) continue;

				renderer->Render(_data, RenderingPath::Forward); // we render the transparent renderers last with forward rendering
			}
		}
	}
}

void GameObjectManager::onGraphicsUpdate(UniformData _data)
{
	for (auto& graphicsGameObject : m_graphicsEntities)
	{
		// Apply other uniform data to the shader
		graphicsGameObject->onGraphicsUpdate(_data);
	}
}

void GameObjectManager::onFixedUpdate(float fixedDeltaTime)
{
	for (auto&& [id, entities] : m_gameObjects)
	{
		for (auto&& [ptr, GameObject] : entities)
		{
			GameObject->onFixedUpdate(fixedDeltaTime);
		}
	}

	for (auto&& [id, gameObjects] : m_gameObjects)
	{
		for (auto&& [ptr, gameObject] : gameObjects)
		{
			Rigidbody* rb = gameObject->getComponent<Rigidbody>();
			if(rb)
			{
				rb->onFixedUpdate(fixedDeltaTime);
			}
		}
	}
}

std::vector<GraphicsGameObject*> GameObjectManager::getGraphicsEntities() const
{
	return m_graphicsEntities;
}

std::vector<Camera*> GameObjectManager::getCameras() const
{
	std::vector<Camera*> cameras;

	// Iterate over all game objects and check if they have a Camera component
	for (auto&& [id, gameObjects] : m_gameObjects)
	{
		for (auto&& [ptr, gameObject] : gameObjects)
		{
			Camera* camera = gameObject->getComponent<Camera>();
			if (camera) {
				cameras.push_back(camera);
			}
		}
	}

	return cameras;
}

void GameObjectManager::clearGameObjects()
{
	// Clear the entities in the map
	m_gameObjects.clear();

	// Clear the graphics entities vector
	m_graphicsEntities.clear();

	// Clear the cameras vector
	m_cameras.clear();

	m_entitiesToDestroy.clear();
}
