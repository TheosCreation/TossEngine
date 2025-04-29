/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2025 Media Design School
File Name : PhysicsMaterial.h
Description : A wrapper class for physics material properties (friction, bounciness) editable in the inspector.
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
***/

#pragma once

#include "Resource.h"

/**
 * @class PhysicsMaterial
 * @brief A physics material resource holding friction and bounciness properties for simulation.
 */
class TOSSENGINE_API PhysicsMaterial : public Resource
{
public:
    /**
     * @brief Constructor for creating a physics material from a description.
     * @param desc The physics material description.
     * @param uniqueId The unique resource ID.
     * @param manager Pointer to the resource manager.
     */
    PhysicsMaterial(const PhysicsMaterialDesc& desc, const std::string& uniqueId, ResourceManager* manager);

    /**
     * @brief Constructor for loading an existing physics material resource.
     * @param uid The unique resource ID.
     * @param mgr Pointer to the resource manager.
     */
    PhysicsMaterial(const std::string& uid, ResourceManager* mgr);

    /**
     * @brief Draws the inspector GUI for editing the physics material.
     */
    void OnInspectorGUI() override;

    /**
     * @brief Deletes the physics material.
     * @param deleteSelf Whether to remove it from the resource manager.
     * @return True if deleted successfully, false otherwise.
     */
    bool Delete(bool deleteSelf = true) override;

    // --- Getters and Setters for material properties ---

    float getStaticFriction() const { return m_staticFriction; }
    void setStaticFriction(float friction) { m_staticFriction = friction; }

    float getDynamicFriction() const { return m_dynamicFriction; }
    void setDynamicFriction(float friction) { m_dynamicFriction = friction; }

    float getBounciness() const { return m_bounciness; }
    void setBounciness(float bounciness) { m_bounciness = bounciness; }

private:
    float m_staticFriction = 1.0f; //!< Static friction coefficient.
    float m_dynamicFriction = 1.0f; //!< Dynamic friction coefficient.
    float m_bounciness = 1.0f; //!< Bounciness (restitution).

    SERIALIZABLE_MEMBERS(m_staticFriction, m_dynamicFriction, m_bounciness)
};

REGISTER_RESOURCE(PhysicsMaterial)

// --- JSON Serialization ---

inline void to_json(json& j, const PhysicsMaterialPtr& material) {
    if (material) {
        j = json{ { "id", material->getUniqueID() } };
    }
    else {
        j = nullptr;
    }
}

inline void from_json(const json& j, PhysicsMaterialPtr& material) {
    if (j.contains("id") && !j["id"].is_null())
        material = ResourceManager::GetInstance().get<PhysicsMaterial>(j["id"].get<std::string>());
}