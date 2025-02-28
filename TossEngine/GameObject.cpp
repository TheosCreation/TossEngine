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
#include "ComponentRegistry.h"
#include "Component.h"

GameObject::GameObject()
{
}

GameObject::GameObject(const GameObject& other)
{
    m_transform = other.m_transform;
    m_id = other.m_id;
    m_gameObjectManager = other.m_gameObjectManager;

    // Deep copy all components
    for (const auto& pair : other.m_components)
    {
        // Clone the component and add it to the new GameObject
        //Component* clonedComponent = pair.second->Clone();
        //if (clonedComponent)
        //{
        //    clonedComponent->setOwner(this); // Set the owner of the cloned component
        //    m_components.emplace(pair.first, clonedComponent); // Add the cloned component to the map
        //}
    }
}

GameObject::~GameObject()
{
	for (auto& pair : m_components) {
		pair.second->onDestroy();
	}
    for (auto& pair : m_components)
    {
        delete pair.second; // Free allocated memory
    }
    m_components.clear();
}

json GameObject::serialize() const
{
    json componentsJson = json::array();
    for (const auto& pair : m_components)
    {
        componentsJson.push_back(pair.second->serialize());
    }

    return {
        {"type", getClassName(typeid(*this))}, // Use typeid to get the class name
        {"transform", {
            {"position", {m_transform.position.x, m_transform.position.y, m_transform.position.z}},
            {"rotation", {m_transform.rotation.x, m_transform.rotation.y, m_transform.rotation.z, m_transform.rotation.w}},
            {"scale", {m_transform.scale.x, m_transform.scale.y, m_transform.scale.z}}
        }},
        {"components", componentsJson}
    };
}

void GameObject::deserialize(const json& data) 
{
    if (data.contains("transform"))
    {
        auto transformData = data["transform"];
        if (transformData.contains("position"))
        {
            auto pos = transformData["position"];
            m_transform.position = Vector3(pos[0], pos[1], pos[2]);
        }
        if (transformData.contains("rotation"))
        {
            auto rot = transformData["rotation"];
            m_transform.rotation = Quaternion(rot[3], rot[0], rot[1], rot[2]);
        }
        if (transformData.contains("scale"))
        {
            auto scl = transformData["scale"];
            m_transform.scale = Vector3(scl[0], scl[1], scl[2]);
        }
    }

    if (data.contains("components"))
    {
        for (const auto& componentData : data["components"])
        {
            std::string componentType = componentData["type"];
            auto component = ComponentRegistry::GetInstance().createComponent(componentType);
            if (component)
            {
                component->setOwner(this);
                component->onCreate();
                component->deserialize(componentData); 
                m_components.emplace(std::type_index(typeid(*component)), component);
            }
        }
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

void GameObject::onCreate()
{
}

void GameObject::onFixedUpdate(float fixedDeltaTime)
{
    for (auto& pair : m_components) {
        pair.second->onFixedUpdate(fixedDeltaTime);
    }
}

void GameObject::onUpdate(float deltaTime)
{
    for (auto& pair : m_components) {
        pair.second->onUpdate(deltaTime);
    }
}

void GameObject::onLateUpdate(float deltaTime)
{
}

//Component* GameObject::addCSharpComponent(const std::string& typeName)
//{
//	Component* nativeComponent = MonoIntegration::CreateCSharpComponent(typeName.c_str());
//	if (nativeComponent)
//	{
//		nativeComponent->setOwner(this);
//		m_components[std::type_index(typeid(*nativeComponent))] = std::unique_ptr<Component>(nativeComponent);
//	}
//	return nativeComponent;
//}

void GameObject::setGameObjectManager(GameObjectManager* gameObjectManager)
{
	// Set the GameObjectManager managing this GameObject
	m_gameObjectManager = gameObjectManager;
}

GameObjectManager* GameObject::getGameObjectManager()
{
	return m_gameObjectManager;
}