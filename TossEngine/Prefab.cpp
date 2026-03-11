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
    rootJ["m_path"] = m_path;
    return rootJ;
}

void Prefab::recurseDeserialize(GameObject* parentGO, const json& data)
{
    if (!data.contains("children"))
    {
        return;
    }

    for (const json& childJson : data["children"])
    {
        GameObject* childGameObject = new GameObject();

        childGameObject->deserialize(childJson);
        childGameObject->onCreate();
        childGameObject->onCreateLate();

        childGameObject->m_transform.SetParent(&parentGO->m_transform, false);

        recurseDeserialize(childGameObject, childJson);
    }
}

void Prefab::deserialize(const nlohmann::json& data)
{
    GameObject::deserialize(data);

    if (data.contains("m_path") && data["m_path"].is_string())
    {
        m_path = data["m_path"].get<std::string>();
    }

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
