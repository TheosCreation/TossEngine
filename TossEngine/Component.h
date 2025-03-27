#pragma once
#include "Serializable.h"
#include "ISelectable.h"
#include "ComponentRegistry.h"
#include <imgui.h>
#include <ImGuizmo.h>

class GameObject;

class TOSSENGINE_API Component : public Serializable, public ISelectable
{
public:
    //Component(const Component& other);
    //
	GameObject* getOwner();
	string getName();
	void setOwner(GameObject* gameObject);

    /**
     * @brief Called when the component is created.
     * Can be overridden by derived classes to perform initialization.
     */
    virtual void onCreate() {
    }

    virtual void onStart() {
    }

    virtual void onLateStart() {
    }

    virtual void onUpdate(float deltaTime) {
    }

    virtual void onUpdateInternal() {
    }

    virtual void onFixedUpdate(float fixedDeltaTime) {
    }

    virtual void onDestroy() {
    }

    virtual void OnInspectorGUI() 
    {
    }

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