#pragma once
#include "Resource.h"

// A wrapper for the Physics Material to be used in the inspector
class PhysicsMaterial : public Resource
{
public:
    // Constructor with parameters for setting material properties
    PhysicsMaterial(const PhysicsMaterialDesc& desc, const std::string& uniqueId, ResourceManager* manager);

    void OnInspectorGUI() override;
    bool Delete(bool deleteSelf = true) override;

    // Getter and setter for material properties
    float getStaticFriction() const { return m_staticFriction; }
    void setStaticFriction(float friction) { m_staticFriction = friction; }

    float getDynamicFriction() const { return m_dynamicFriction; }
    void setDynamicFriction(float friction) { m_dynamicFriction = friction; }

    float getBounciness() const { return m_bounciness; }
    void setBounciness(float bounciness) { m_bounciness = bounciness; }

    // For inspector/serialization
    json serialize() const;
    void deserialize(const json& data);

private:
    // Properties of the material
    float m_staticFriction = 1.0f;
    float m_dynamicFriction = 1.0f;
    float m_bounciness = 1.0f;
};
