#include "Gun.h"

json Gun::serialize() const
{
    json data;
    data["type"] = getClassName(typeid(*this)); // Store the component type
    if (m_projectile)
    {
        data["projectilePrefab"] = m_projectile->getId();
    }

    return data;
}

void Gun::deserialize(const json& data)
{
    if (data.contains("projectilePrefab"))
    {
        m_projectile = ResourceManager::GetInstance().getPrefab(data["projectilePrefab"].get<string>());
    }
}

void Gun::OnInspectorGUI()
{
    Component::OnInspectorGUI();

    ResourceDropdownField(m_projectile, "Projectile");
}

void Gun::onUpdate()
{

}
