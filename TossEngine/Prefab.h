#pragma once
#include "GameObject.h"
#include "Resource.h"

class Prefab : public GameObject, public Resource
{
public:
    Prefab(const std::string& uniqueID, ResourceManager* manager);
    json serialize() const override;
    void deserialize(const json& data) override;
    void OnInspectorGUI() override;
    reactphysics3d::PhysicsWorld* getWorld() override;

    GameObject* Instantiate() const {
        // Serialize the prefab into JSON.
        json data = this->serialize();
        // Create a new empty game object.
        GameObject* newObject = new GameObject();
        // Deserialize the saved data into the new object.
        newObject->deserialize(data);
        return newObject;
    }
};

