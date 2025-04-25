#pragma once
#include "GameObject.h"
#include "Resource.h"

class TOSSENGINE_API Prefab : public GameObject, public Resource
{
public:
    Prefab(const std::string& uid, ResourceManager* mgr);
    json serialize() const override;
    void deserialize(const json& data) override;
    void OnInspectorGUI() override;
    reactphysics3d::PhysicsWorld* getWorld() override;

    void onCreate() override;
    void onCreateLate() override;
    void onDestroy() override;

    GameObjectPtr Instantiate() const {
        // Serialize the prefab into JSON.
        json data = this->serialize();
        // Create a new empty game object.
        GameObjectPtr newObject = std::make_shared<GameObject>();
        // Deserialize the saved data into the new object.
        newObject->deserialize(data);
        return newObject;
    }


    static void recurseSerialize(const GameObject* go, json& out);
    static void recurseDeserialize(GameObject* parentGO, const json& data);
};


inline void to_json(json& j, PrefabPtr const& prefab) {
    if (prefab) 
    {
        j = json{ { "id", prefab->getUniqueID() } };
    }
    else {
        j = nullptr;
    }
}

inline void from_json(json const& j, PrefabPtr& prefab) {
    if (j.contains("id")) prefab = ResourceManager::GetInstance().getPrefab(j["id"].get<string>());
}
