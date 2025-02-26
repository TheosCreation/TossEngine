/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2024 Media Design School
File Name : GameObject.cpp
Description : GameObject class that represents an object for OpenGl with its own update and onCreate functions.
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
**/

#include "GameObject.h"
#include "GameObjectManager.h"

GameObject::GameObject()
{
}

GameObject::~GameObject()
{
	for (auto& pair : m_components) {
		pair.second->onDestroy();
	}
}

size_t GameObject::getId()
{
	return m_id;
}

void GameObject::setId(size_t id)
{
	m_id = id;
}

void GameObject::release()
{
	// Remove this GameObject from the GameObjectManager
	m_gameObjectManager->removeGameObject(this);
}

void GameObject::setGameObjectManager(GameObjectManager* gameObjectManager)
{
	// Set the GameObjectManager managing this GameObject
	m_gameObjectManager = gameObjectManager;
}

GameObjectManager* GameObject::getGameObjectManager()
{
	return m_gameObjectManager;
}