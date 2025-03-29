#include "PhysicsMaterial.h"

PhysicsMaterial::PhysicsMaterial(const PhysicsMaterialDesc& desc, const string& uniqueId, ResourceManager* manager) : Resource(uniqueId, uniqueId, manager)
{
    desc.staticFriction;
    desc.dynamicFriction;
    desc.bounciness;

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