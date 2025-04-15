#include "Prefab.h"
#include "Physics.h"

Prefab::Prefab(const std::string& uniqueID, ResourceManager* manager) : Resource("", uniqueID, manager)
{
}

void Prefab::OnInspectorGUI()
{
    GameObject::OnInspectorGUI();
}

reactphysics3d::PhysicsWorld* Prefab::getWorld()
{
    return Physics::GetInstance().GetPrefabWorld();
}
