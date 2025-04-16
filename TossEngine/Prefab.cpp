#include "Prefab.h"
#include "Physics.h"

Prefab::Prefab(const std::string& uniqueID, ResourceManager* manager) : Resource("", uniqueID, manager)
{
}

json Prefab::serialize() const
{
    return GameObject::serialize();
}

void Prefab::deserialize(const json& data)
{
    GameObject::deserialize(data);
}

void Prefab::OnInspectorGUI()
{
    GameObject::OnInspectorGUI();
}

reactphysics3d::PhysicsWorld* Prefab::getWorld()
{
    return Physics::GetInstance().GetPrefabWorld();
}
