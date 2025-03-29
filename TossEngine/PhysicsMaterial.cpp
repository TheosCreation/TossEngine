#include "PhysicsMaterial.h"
#include "ResourceManager.h"

PhysicsMaterial::PhysicsMaterial(const PhysicsMaterialDesc& desc, const string& uniqueId, ResourceManager* manager) : Resource(uniqueId, uniqueId, manager)
{
    m_staticFriction = desc.staticFriction;
    m_dynamicFriction = desc.dynamicFriction;
    m_bounciness = desc.bounciness;
}

void PhysicsMaterial::OnInspectorGUI()
{
    ImGui::Text(("Physics Material Inspector - ID: " + m_uniqueID).c_str());
    ImGui::Separator();

    ImGui::DragFloat("Static Friction", &m_staticFriction, 0.1f);
    ImGui::DragFloat("Dynamic Friction", &m_dynamicFriction, 0.1f);
    ImGui::DragFloat("Bounciness", &m_bounciness, 0.1f);
}

bool PhysicsMaterial::Delete(bool deleteSelf)
{
    if (m_resourceManager)
    {
        m_resourceManager->DeleteResource(m_uniqueID);
        return true;
    }

    return false;
}

// Serialize the material properties into a JSON object for saving/loading
json PhysicsMaterial::serialize() const
{
    json data;
    data["staticFriction"] = m_staticFriction;
    data["dynamicFriction"] = m_dynamicFriction;
    data["bounciness"] = m_bounciness;
    return data;
}

// Deserialize material properties from a JSON object
void PhysicsMaterial::deserialize(const json& data)
{
    if (data.contains("staticFriction"))
        m_staticFriction = data["staticFriction"].get<float>();

    if (data.contains("dynamicFriction"))
        m_dynamicFriction = data["dynamicFriction"].get<float>();

    if (data.contains("bounciness"))
        m_bounciness = data["bounciness"].get<float>();
}