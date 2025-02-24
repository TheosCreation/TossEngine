#include "Component.h"

GameObject* Component::getOwner()
{
    return m_owner;
}

void Component::setOwner(GameObject* gameObject)
{
    m_owner = gameObject;
}
