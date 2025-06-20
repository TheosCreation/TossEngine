#include "Prefab.h"
#include "Physics.h"

Prefab::Prefab(const std::string& uid, ResourceManager* mgr) : Resource(uid, mgr)
{
}

void Prefab::recurseSerialize(const GameObject* go, json& out)
{
    out = go->GameObject::serialize();

    // then, if it has children in its transform, recurse:
    if (!go->m_transform.children.empty())
    {
        out["children"] = json::array();
        for (Transform* childT : go->m_transform.children)
        {
            GameObject* childGO = childT->gameObject;
            json childJ;
            recurseSerialize(childGO, childJ);
            out["children"].push_back(std::move(childJ));
        }
    }
}

json Prefab::serialize() const
{
    json rootJ;
    recurseSerialize(this, rootJ);
    return rootJ;
}

void Prefab::recurseDeserialize(GameObject* parentGO, const json& data)
{
    //children dont work yet for prefabs
    // for each declared child:
    if (data.contains("children"))
    {
        for (auto& childJson : data["children"])
        {
            GameObject* childGO = new GameObject();

            // populate its own fields & components:
            childGO->deserialize(childJson);
            childGO->onCreate();
            childGO->onCreateLate();

            // hook up transform parent ? child
            childGO->m_transform.SetParent(&parentGO->m_transform, false);

            // recurse any deeper grandchildren
            recurseDeserialize(childGO, childJson);
        }
    }
}

void Prefab::deserialize(const nlohmann::json& data)
{
    // 1) rebuild root node
    GameObject::deserialize(data);

    // 2) then rebuild all children under that root
    recurseDeserialize(this, data);
}

void Prefab::OnInspectorGUI()
{
    GameObject::OnInspectorGUI();
}

reactphysics3d::PhysicsWorld* Prefab::getWorld()
{
    return Physics::GetInstance().GetPrefabWorld();
}

void Prefab::onCreate()
{
    GameObject::onCreate();
    Resource::onCreate();
}

void Prefab::onCreateLate()
{
    GameObject::onCreateLate();
    Resource::onCreateLate();
}

void Prefab::onDestroy()
{
    GameObject::onDestroy();
    Resource::onDestroy();
}
