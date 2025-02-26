#pragma once
#include "Serializable.h"

// Add the callback delegates for C#
extern "C" {
    typedef void (*OnCreateCallback)();
    typedef void (*OnUpdateCallback)(float);
    typedef void (*OnFixedUpdateCallback)(float);
    typedef void (*OnDestroyCallback)();

    // Set callbacks for each lifecycle method
    TOSSENGINE_API void SetOnCreateCallback(Component* component, OnCreateCallback callback);
    TOSSENGINE_API void SetOnUpdateCallback(Component* component, OnUpdateCallback callback);
    TOSSENGINE_API void SetOnFixedUpdateCallback(Component* component, OnFixedUpdateCallback callback);
    TOSSENGINE_API void SetOnDestroyCallback(Component* component, OnDestroyCallback callback);
}

class GameObject;

class TOSSENGINE_API Component : public Serializable
{
public:
	GameObject* getOwner();
	void setOwner(GameObject* gameObject);

    /**
     * @brief Called when the component is created.
     * Can be overridden by derived classes to perform initialization.
     */
    virtual void onCreate() {
        if (onCreateCallback) {
            onCreateCallback();  // Call the C# override
        }
    }
    virtual void onUpdate(float deltaTime) {
        if (onUpdateCallback) {
            onUpdateCallback(deltaTime);  // Call the C# override
        }
    }

    virtual void onFixedUpdate(float fixedDeltaTime) {
        if (onFixedUpdateCallback) {
            onFixedUpdateCallback(fixedDeltaTime);  // Call the C# override
        }
    }

    virtual void onDestroy() {
        if (onDestroyCallback) {
            onDestroyCallback();  // Call the C# override
        }
    }
    /**
     * @brief Called every frame after all Update functions have been called.
     * Can be overridden by derived classes to implement custom behavior.
     * @param deltaTime The time elapsed since the last frame.
     */
    virtual void onLateUpdate(float deltaTime) {}

    // Callbacks from C#
    OnCreateCallback onCreateCallback;
    OnUpdateCallback onUpdateCallback;
    OnFixedUpdateCallback onFixedUpdateCallback;
    OnDestroyCallback onDestroyCallback;

protected:
	GameObject* m_owner = nullptr;
};

// Set callbacks implementation
extern "C" TOSSENGINE_API void SetOnCreateCallback(Component* component, OnCreateCallback callback) {
    component->onCreateCallback = callback;
}

extern "C" TOSSENGINE_API void SetOnUpdateCallback(Component* component, OnUpdateCallback callback) {
    component->onUpdateCallback = callback;
}

extern "C" TOSSENGINE_API void SetOnFixedUpdateCallback(Component* component, OnFixedUpdateCallback callback) {
    component->onFixedUpdateCallback = callback;
}

extern "C" TOSSENGINE_API void SetOnDestroyCallback(Component* component, OnDestroyCallback callback) {
    component->onDestroyCallback = callback;
}