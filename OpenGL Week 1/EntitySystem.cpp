/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2024 Media Design School
File Name : EntitySystem.cpp
Description : Entity system is a container and controller of the entities for the game
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
**/

#include "EntitySystem.h"
#include "Entity.h"
#include "Camera.h"
#include "GraphicsEntity.h"
#include "GraphicsEngine.h"
#include "MeshRenderer.h"
#include "RigidBody.h"

GameObjectManager::GameObjectManager()
{
	m_entityFactory = std::make_unique<EntityFactory>();
}

GameObjectManager::GameObjectManager(Scene* scene)
{
	m_entityFactory = std::make_unique<EntityFactory>();
	m_scene = scene;
}

GameObjectManager::~GameObjectManager()
{
}

Scene* GameObjectManager::getScene()
{
	return m_scene;
}

bool GameObjectManager::createGameObjectInternal(GameObject* entity, size_t id)
{
	auto ptr = std::unique_ptr<GameObject>(entity);

	// Check if the entity is of type GraphicsEntity
	if (dynamic_cast<GraphicsEntity*>(entity)) {
		m_graphicsEntities.push_back(static_cast<GraphicsEntity*>(entity));
	}

	// Add the entity to the internal map
	m_gameObjects[id].emplace(entity, std::move(ptr));

	// Initialize the entity
	entity->setId(id);
	entity->setGameObjectManager(this);
	entity->onCreate();

	return true;
}

void GameObjectManager::removeEntity(GameObject* entity)
{
	m_entitiesToDestroy.emplace(entity);
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

	if (!m_entityFactory)
	{
		std::cerr << "Error: m_entityFactory is null!" << std::endl;
		return;
	}

	for (const auto& entityData : sceneData["entities"])
	{
		if (!entityData.contains("type") || !entityData["type"].is_string())
		{
			std::cerr << "Error: Entity does not have a valid 'type' field!" << std::endl;
			continue;
		}

		std::string type = entityData["type"];
		std::cout << "Trying to create entity of type: " << type << std::endl;

		GameObject* entity = m_entityFactory->createEntity(type);
		if (entity)
		{
			std::cout << "Entity created successfully: " << type << std::endl;
			entity->deserialize(entityData);
			std::cout << "Entity hash code: " << typeid(*entity).hash_code() << std::endl;
			createGameObjectInternal(entity, typeid(*entity).hash_code());
		}
		else
		{
			std::cerr << "Unknown entity type: " << type << std::endl;
		}
	}
}


void GameObjectManager::saveEntitiesToFile(const std::string& filePath)
{
	json sceneData;
	sceneData["entities"] = json::array();

	for (auto&& [id, entities] : m_gameObjects)
	{
		for (auto&& [ptr, entity] : entities)
		{
			if (entity)
			{
				sceneData["entities"].push_back(entity->serialize());
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
		for (auto&& [ptr, entity] : entities)
		{
			entity->onUpdate(deltaTime);
		}
	}
}

void GameObjectManager::onLateUpdate(float deltaTime)
{
	for (auto&& [id, entities] : m_gameObjects)
	{
		for (auto&& [ptr, entity] : entities)
		{
			entity->onLateUpdate(deltaTime);
		}
	}
}

void GameObjectManager::onShadowPass(int index)
{
	//for (auto& graphicsEntity : m_graphicsEntities)
	//{
	//	graphicsEntity->onShadowPass(index);
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
	for (auto&& [id, gameObjects] : m_gameObjects)
	{
		for (auto&& [ptr, gameObject] : gameObjects)
		{
			MeshRenderer* renderer = gameObject->getComponent<MeshRenderer>();
			if (renderer)
			{
				renderer->Render(_data);
			}
		}
	}
}

void GameObjectManager::onGeometryPass(UniformData _data)
{
	for (auto& graphicsEntity : m_graphicsEntities)
	{
		if (graphicsEntity->getTransparency() < 1.0f) continue; // if the renderer is transparent we skip it

		graphicsEntity->onGeometryPass(_data);
	}
}

void GameObjectManager::onTransparencyPass(UniformData _data)
{
	for (auto& graphicsEntity : m_graphicsEntities)
	{
		if (graphicsEntity->getTransparency() == 1.0f) continue; // if the renderer is opaque we skip it

		graphicsEntity->onGraphicsUpdate(_data);
	}


	for (auto&& [id, gameObjects] : m_gameObjects)
	{
		for (auto&& [ptr, gameObject] : gameObjects)
		{
			MeshRenderer* renderer = gameObject->getComponent<MeshRenderer>();
			if (renderer)
			{
				if(renderer->GetAlpha()  == 1.0f) continue;

				renderer->Render(_data);
			}
		}
	}
}

void GameObjectManager::onGraphicsUpdate(UniformData _data)
{
	for (auto& graphicsEntity : m_graphicsEntities)
	{
		// Apply other uniform data to the shader
		graphicsEntity->onGraphicsUpdate(_data);
	}
}

void GameObjectManager::onFixedUpdate(float fixedDeltaTime)
{
	for (auto&& [id, entities] : m_gameObjects)
	{
		for (auto&& [ptr, entity] : entities)
		{
			entity->onFixedUpdate(fixedDeltaTime);
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

std::vector<GraphicsEntity*> GameObjectManager::getGraphicsEntities() const
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
