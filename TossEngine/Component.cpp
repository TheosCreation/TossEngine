#include "Component.h"

GameObject* Component::getOwner()
{
    return m_owner;
}

void Component::setOwner(GameObject* gameObject)
{
    m_owner = gameObject;
}

TOSSENGINE_API Component* CreateComponent()
{
    return new Component();
}

TOSSENGINE_API void DestroyComponent(Component* component)
{
    delete component;
}
