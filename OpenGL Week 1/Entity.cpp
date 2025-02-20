/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2024 Media Design School
File Name : Entity.cpp
Description : Entity class that represents an object for OpenGl with its own update and onCreate functions.
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
**/

#include "Entity.h"
#include "EntitySystem.h"

GameObject::GameObject()
{
}

GameObject::~GameObject()
{
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
	// Remove this entity from the EntitySystem
	m_entitySystem->removeEntity(this);
}

void GameObject::setEntitySystem(GameObjectManager* entitySystem)
{
	// Set the EntitySystem managing this entity
	m_entitySystem = entitySystem;
}

GameObjectManager* GameObject::getGameObjectManager()
{
	return m_entitySystem;
}