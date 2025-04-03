#pragma once
#include "Serializable.h"
#include "ISelectable.h"
#include "ComponentRegistry.h"
#include <imgui.h>

class GameObject;
class Collider;

class TOSSENGINE_API Component : public Serializable, public ISelectable
{
public:
    GameObject* getOwner();
    string getName();
    void setOwner(GameObject* gameObject);

    virtual json serialize() const //do call Component::serialize() first when overriding this class
    {
        return {
            {"type", getClassName(typeid(*this))} // used to identify component type
        };
    }

    /**
     * @brief Called when the component is created.
     * Can be overridden by derived classes to perform initialization.
     */
    virtual void onCreate() {}

    /**
     * @brief Called when the component is created and after deserilization
     * Can be overridden by derived classes to perform initialization.
     */
    virtual void onLateCreate() {}

    virtual void onStart() {}

    virtual void onLateStart() {}

    virtual void onUpdate(float deltaTime) {}

    virtual void onUpdateInternal() {}

    virtual void onFixedUpdate(float fixedDeltaTime) {}

    virtual void onDestroy() { }

    virtual void OnInspectorGUI()
    {
        string inspectorName = getClassName(typeid(*this)) + " Inspector - ID: %p";
        ImGui::Text(inspectorName.c_str(), this);
        ImGui::Separator();
    }
    virtual void OnGameObjectSelected(){}
    virtual void OnGameObjectDeSelected() {}

    virtual void onCollisionEnter(Collider* other) {}
    virtual void onCollisionExit(Collider* other) {}
    virtual void onTriggerEnter(Collider* other) {}
    virtual void onTriggerExit(Collider* other) {}

    bool Delete(bool deleteSelf = true);
    void Destroy(GameObject* objectToDestroy);

    /**
     * @brief Called every frame after all Update functions have been called.
     * Can be overridden by derived classes to implement custom behavior.
     * @param deltaTime The time elapsed since the last frame.
     */
    virtual void onLateUpdate(float deltaTime) {}

protected:
    GameObject* m_owner = nullptr;
};