#include "Component.h"

GameObject* Component::getOwner()
{
    return m_gameObjectOwnerRef;
}

void Component::setOwner(GameObject* gameObject)
{
    m_gameObjectOwnerRef = gameObject;
}
