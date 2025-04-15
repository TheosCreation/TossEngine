#pragma once
#include "Serializable.h"
#include "ISelectable.h"
#include "ComponentRegistry.h"
#include "ResourceManager.h"
#include "Resource.h"
#include <imgui.h>

class GameObject;
class Collider;

class TOSSENGINE_API Component : public Serializable, public ISelectable
{
public:
    Component() = default;
    virtual ~Component() = default;

    GameObject* getOwner() const;
    string getName();
    void setOwner(GameObject* gameObject);

    virtual json serialize() const override
    {
        return {
            {"type", getClassName(typeid(*this))} // used to identify component type
        };
    }

    /**
     * @brief Called when the component is created and before deserialization.
     * Can be overridden by derived classes to perform initialization.
     */
    virtual void onCreate() {}

    /**
     * @brief Called when the component is created and after deserialization
     * Can be overridden by derived classes to perform initialization.
     */
    virtual void onCreateLate() {}

    virtual void onStart() {}

    virtual void onLateStart() {}

    virtual void onUpdate() {}

    virtual void onUpdateInternal() {}

    virtual void onFixedUpdate() {}

    virtual void onDestroy() { }

    virtual void OnInspectorGUI() override
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

    virtual bool Delete(bool deleteSelf = true) override;
    static void Destroy(GameObject* objectToDestroy);

    /**
     * @brief Called every frame after all Update functions have been called.
     * Can be overridden by derived classes to implement custom behavior.
     */
    virtual void onLateUpdate() {}

    Transform& getTransform() const;

protected:
    GameObject* m_owner = nullptr;
};