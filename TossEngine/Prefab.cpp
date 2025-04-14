#include "Prefab.h"

Prefab::Prefab(const std::string& uniqueID, ResourceManager* manager) : Resource("", uniqueID, manager)
{
}

void Prefab::OnInspectorGUI()
{
    GameObject::OnInspectorGUI();
}