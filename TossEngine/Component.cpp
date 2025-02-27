#include "Component.h"

GameObject* Component::getOwner()
{
    return m_owner;
}

void Component::setOwner(GameObject* gameObject)
{
    m_owner = gameObject;
}

TOSSENGINE_API void SetCSharpComponentCallbacks(Component* component, OnCreateCallback onCreate, OnUpdateCallback onUpdate, OnFixedUpdateCallback onFixedUpdate, OnDestroyCallback onDestroy)
{
    if (component) {
        component->onCreateCallback = onCreate;
        component->onUpdateCallback = onUpdate;
        component->onFixedUpdateCallback = onFixedUpdate;
        component->onDestroyCallback = onDestroy;
    }
}
