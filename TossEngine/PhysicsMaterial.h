#pragma once
#include "Resource.h"

// A wrapper for the Physics Material to be used in the inspector
class PhysicsMaterial : public Resource
{
public:
    // Constructor with parameters for setting material properties
    PhysicsMaterial(const PhysicsMaterialDesc& desc, const std::string& uniqueId, ResourceManager* manager);
    PhysicsMaterial(const std::string& uid, ResourceManager* mgr);

    void OnInspectorGUI() override;
    bool Delete(bool deleteSelf = true) override;

    // Getter and setter for material properties
    float getStaticFriction() const { return m_staticFriction; }
    void setStaticFriction(float friction) { m_staticFriction = friction; }

    float getDynamicFriction() const { return m_dynamicFriction; }
    void setDynamicFriction(float friction) { m_dynamicFriction = friction; }

    float getBounciness() const { return m_bounciness; }
    void setBounciness(float bounciness) { m_bounciness = bounciness; }

private:
    // Properties of the material
    float m_staticFriction = 1.0f;
    float m_dynamicFriction = 1.0f;
    float m_bounciness = 1.0f;

    SERIALIZABLE_MEMBERS(m_staticFriction, m_dynamicFriction, m_bounciness)
};
REGISTER_RESOURCE(PhysicsMaterial)

inline void to_json(json& j, PhysicsMaterialPtr const& material) {
    if (material)
    {
        j = json{ { "id", material->getUniqueID() } };
    }
    else {
        j = nullptr;
    }
}

inline void from_json(json const& j, PhysicsMaterialPtr& material) {
    if (j.contains("id")) material = ResourceManager::GetInstance().get<PhysicsMaterial>(j["id"].get<string>());
}