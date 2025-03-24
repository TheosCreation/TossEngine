#include "Component.h"
#include "GameObject.h"
#include "GameObjectManager.h"

//Component::Component(const Component& other)
//{
//}

GameObject* Component::getOwner()
{
    return m_owner;
}

string Component::getName()
{
    return getClassName(typeid(*this));
}

void Component::setOwner(GameObject* gameObject)
{
    m_owner = gameObject;
}

void Component::Destroy(GameObject* objectToDestroy)
{
    objectToDestroy->release();
}