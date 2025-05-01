/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2025 Media Design School
File Name : GameObject.h
Description : Represents a game entity with transform, components, lifecycle events,
              and serialization support. Forms the base unit of the TossEngine scene graph.
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
***/

#pragma once

#include "Utils.h"
#include "Math.h"
#include "Serializable.h"
#include "Component.h"
#include "GameObjectManager.h"
#include "ISelectable.h"
#include "LayerManager.h"

namespace reactphysics3d
{
    class PhysicsWorld;
}

class MissingComponent;
class Collider;

/**
 * @class GameObject
 * @brief Core entity class representing a runtime object with transform, components, and serialization support.
 */
class TOSSENGINE_API GameObject : public Serializable, public ISelectable
{
public:
    GameObject();
    GameObject(const GameObject& other); // Copy constructor
    virtual ~GameObject();

    std::string name = "GameObject";
    std::string tag = "";
    std::string m_layer = "Default";
    Transform m_transform;

    bool isDestroyed = false;
    bool isActive = true;
    bool isGameRunning = false;

    // --- Lifecycle ---

    virtual void onCreate();
    virtual void onCreateLate();
    virtual void onStart();
    virtual void onLateStart();
    virtual void onFixedUpdate();
    virtual void onUpdate();
    virtual void onLateUpdate();
    virtual void onDestroy();

    virtual void onLocalScaleChanged(Vector3 previousScale);

    void onUpdateInternal();

    // --- Inspector ---
    virtual void OnInspectorGUI() override;
    virtual void OnSelect() override;
    virtual void OnDeSelect() override;
    void drawComponentInspector(Component* comp, const std::string& displayName);

    // --- Component System ---

    /**
     * @brief Adds a component of the given type.
     *        Automatically calls lifecycle hooks (onCreate, onStart, etc).
     * @return Pointer to the new component.
     */
    template <typename Component>
    Component* addComponent()
    {
        auto component = new Component();
        std::type_index typeIndex(typeid(Component));

        component->setOwner(this);
        m_components.emplace(typeIndex, component);
        component->onCreate();

        if (m_finishedCreation)
            component->onCreateLate();
        if (hasStarted) {
            component->onStart();
            component->onLateStart();
        }

        return static_cast<Component*>(m_components[typeIndex]);
    }

    Component* addComponent(std::string componentType, const json& data = nullptr);
    virtual void removeComponent(Component* component);
    virtual void removeMissingComponent(MissingComponent* component);

    template <typename Component>
    Component* getComponent()
    {
        std::type_index typeIndex(typeid(Component));
        auto it = m_components.find(typeIndex);
        if (it != m_components.end())
            return static_cast<Component*>(it->second);
        return nullptr;
    }

    template <typename Component>
    Component* getComponentInChildren()
    {
        if (Component* comp = getComponent<Component>())
            return comp;

        for (Transform* childTransform : m_transform.children)
        {
            if (childTransform->gameObject)
                if (Component* comp = childTransform->gameObject->getComponentInChildren<Component>())
                    return comp;
        }

        return nullptr;
    }

    template <typename Component>
    Component* getComponentInParent()
    {
        GameObject* current = this;

        while (current != nullptr)
        {
            if (Component* comp = current->getComponent<Component>())
                return comp;

            current = current->m_transform.parent ? current->m_transform.parent->gameObject : nullptr;
        }

        return nullptr;
    }

    std::map<std::type_index, Component*>& getAllComponents();
    bool tryDeleteSelectedComponent();

    // --- Serialization ---
    virtual json serialize() const override;
    virtual void deserialize(const json& data) override;

    // --- ID / Layer / Manager Access ---

    size_t getId() const;
    void setId(size_t id);

    LayerBit getLayer() const;

    GameObjectManager* getGameObjectManager() const;
    LightManager* getLightManager() const;
    void setGameObjectManager(GameObjectManager* gameObjectManager);

    bool Delete(bool deleteSelf = true) override;
    bool GetActive() const;

    // --- Physics ---

    virtual reactphysics3d::PhysicsWorld* getWorld();
    void CallOnCollisionEnterCallbacks(Collider* other) const;
    void CallOnCollisionExitCallbacks(Collider* other);
    void CallOnTriggerEnterCallbacks(Collider* other) const;
    void CallOnTriggerExitCallbacks(Collider* other) const;

protected:
    std::map<std::type_index, Component*> m_components;               //!< Active components
    std::map<std::string, MissingComponent*> m_missingComponents;     //!< Components not loaded (e.g., missing DLL)
    std::vector<Component*> componentsToDestroy;
    std::vector<MissingComponent*> missingComponetsToDestroy;

    bool hasStarted = false;
    bool m_finishedCreation = false;

    GameObjectManager* m_gameObjectManager = nullptr;

    Component* selectedComponent = nullptr;

private:
    size_t m_id = 0;          //!< Unique identifier
    char tagBuffer[128];      //!< Internal buffer for tag editing
    Vector3 eulerAngles;      //!< Editor-facing rotation data
};
