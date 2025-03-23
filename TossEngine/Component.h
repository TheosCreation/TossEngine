#pragma once
#include "Serializable.h"

class GameObject;

class TOSSENGINE_API Component : public Serializable
{
public:
    //Component(const Component& other);

	GameObject* getOwner();
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

    virtual void onFixedUpdate(float fixedDeltaTime) {
    }

    virtual void onDestroy() {
    }

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