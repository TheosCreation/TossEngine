#include "Gun.h"

json Gun::serialize() const
{
    json data;
    data["type"] = getClassName(typeid(*this)); // Store the component type
    data["fireRate"] = m_fireRate;
    if (m_projectile)
    {
        data["projectilePrefab"] = m_projectile->getUniqueID();
    }

    return data;
}

void Gun::deserialize(const json& data)
{
    if (data.contains("fireRate"))
    {
        m_fireRate = data["fireRate"].get<float>();
    }

    if (data.contains("projectilePrefab"))
    {
        ResourceManager& resourceManager = ResourceManager::GetInstance();
        m_projectile = resourceManager.getPrefab(data["projectilePrefab"].get<string>());
    }
}

void Gun::OnInspectorGUI()
{
    Component::OnInspectorGUI();

    FloatSliderField("FireRate", m_fireRate);
    ResourceAssignableField(m_projectile, "Projectile");
}

void Gun::onUpdate()
{
    auto& inputManager = InputManager::GetInstance();
    if(inputManager.isMouseDown(MouseButtonLeft) && shootTimer <= 0.0f)
    {
        shootTimer = m_fireRate;
        Debug::Log("Shot Fired");

        //this is how id create a empty game object but i want to create one from the prefab m_projectile
        m_owner->getGameObjectManager()->Instatiate(m_projectile);
    }
    else
    {
        shootTimer -= Time::DeltaTime;
    }
}
