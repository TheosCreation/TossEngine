#include "Component.h"
#include "GameObject.h"
#include "Scene.h"

GameObject* Component::getOwner() const
{
    return m_owner;
}

GameObjectPtr Component::getSharedOwner() const
{
    if (!m_owner) return nullptr;
    Scene* scene = m_owner->getScene();
    if (!scene)
    {
        Debug::LogWarning("Tried to get shared gameobject owner with no scene cannot get it then gameobject name is: " + m_owner->name);
        return nullptr;
    }
    return scene->getGameObject(m_owner->getId());
}

string Component::getName()
{
    return getClassName(typeid(*this));
}

void Component::setOwner(GameObject* gameObject)
{
    m_owner = gameObject;
}

bool Component::Delete(bool deleteSelf)
{
    if (deleteSelf)
    {
        m_owner->removeComponent(this);
    }
    return true;
}

void Component::Destroy(GameObject* objectToDestroy)
{
    objectToDestroy->Delete();
}

Transform& Component::getTransform() const
{
    return m_owner->m_transform;
}