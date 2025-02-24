#pragma once
#include "Serializable.h"

class GameObject;

class Component : Serializable
{
public:
	GameObject* getOwner();
	void setOwner(GameObject* gameObject);

    /**
     * @brief Called when the component is created.
     * Can be overridden by derived classes to perform initialization.
     */
    virtual void onCreate() {}

    /**
     * @brief Called every frame to update the component at a fixed frame rate.
     * Can be overridden by derived classes to implement custom behavior.
     * @param deltaTime The time elapsed since the last frame.
     */
    virtual void onFixedUpdate(float fixedDeltaTime) {}

    /**
     * @brief Called every frame to update the component.
     * Can be overridden by derived classes to implement custom behavior.
     * @param deltaTime The time elapsed since the last frame.
     */
    virtual void onUpdate(float deltaTime) { }

    /**
     * @brief Called every frame after all Update functions have been called.
     * Can be overridden by derived classes to implement custom behavior.
     * @param deltaTime The time elapsed since the last frame.
     */
    virtual void onLateUpdate(float deltaTime) {}

protected:
	GameObject* m_owner;
};

