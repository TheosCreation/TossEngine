#include "PhysicsMaterial.h"
#include "ResourceManager.h"

PhysicsMaterial::PhysicsMaterial(const PhysicsMaterialDesc& desc, const string& uniqueId, ResourceManager* manager) : Resource(uniqueId, uniqueId, manager)
{
    m_staticFriction = desc.staticFriction;
    m_dynamicFriction = desc.dynamicFriction;
    m_bounciness = desc.bounciness;
}

PhysicsMaterial::PhysicsMaterial(const std::string& uid, ResourceManager* mgr) : Resource(uid, mgr)
{
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