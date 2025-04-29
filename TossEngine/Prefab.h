/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2025 Media Design School
File Name : Prefab.h
Description : A prefab resource representing serialized GameObjects that can be instantiated at runtime.
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
***/

#pragma once

#include "GameObject.h"
#include "Resource.h"

/**
 * @class Prefab
 * @brief A GameObject that can be saved as a reusable resource and instantiated.
 */
class TOSSENGINE_API Prefab : public GameObject, public Resource
{
public:
    /**
     * @brief Constructor for loading an existing Prefab resource.
     * @param uid The unique ID of the resource.
     * @param mgr Pointer to the resource manager.
     */
    Prefab(const std::string& uid, ResourceManager* mgr);

    /**
     * @brief Serializes the Prefab into JSON format.
     * @return JSON representation of the Prefab.
     */
    json serialize() const override;

    /**
     * @brief Deserializes the Prefab from JSON data.
     * @param data The JSON data.
     */
    void deserialize(const json& data) override;

    /**
     * @brief Draws the Prefab inspector UI.
     */
    void OnInspectorGUI() override;

    /**
     * @brief Gets the Physics World associated with this Prefab.
     * @return Pointer to the physics world (may be nullptr if prefab is not simulated).
     */
    reactphysics3d::PhysicsWorld* getWorld() override;

    /**
     * @brief Called when the Prefab is created.
     */
    void onCreate() override;

    /**
     * @brief Called after Prefab creation for any deferred setup.
     */
    void onCreateLate() override;

    /**
     * @brief Called when the Prefab is destroyed.
     */
    void onDestroy() override;

    /**
     * @brief Instantiates a copy of this Prefab as a new GameObject.
     * @return A new GameObject instance cloned from this Prefab.
     */
    GameObjectPtr Instantiate() const {
        // Serialize the prefab into JSON.
        json data = this->serialize();
        // Create a new empty game object.
        GameObjectPtr newObject = std::make_shared<GameObject>();
        // Deserialize the saved data into the new object.
        newObject->deserialize(data);
        return newObject;
    }

    /**
     * @brief Recursively serializes a GameObject hierarchy into JSON.
     * @param go The root GameObject to serialize.
     * @param out The JSON output reference.
     */
    static void recurseSerialize(const GameObject* go, json& out);

    /**
     * @brief Recursively deserializes a GameObject hierarchy from JSON.
     * @param parentGO The parent GameObject to attach to.
     * @param data The JSON data to deserialize from.
     */
    static void recurseDeserialize(GameObject* parentGO, const json& data);
};

REGISTER_RESOURCE(Prefab)

// --- JSON Serialization ---

inline void to_json(json& j, const PrefabPtr& prefab) {
    if (prefab) {
        j = json{ { "id", prefab->getUniqueID() } };
    }
    else {
        j = nullptr;
    }
}

inline void from_json(const json& j, PrefabPtr& prefab) {
    if (j.contains("id") && !j["id"].is_null())
        prefab = ResourceManager::GetInstance().get<Prefab>(j["id"].get<std::string>());
}
