/***
DeviousDevs
Auckland
New Zealand
(c) 2026 DeviousDevs
File Name : GameObject.h
Description : Represents a game entity with transform, components, lifecycle events,
              and serialization support. Forms the base unit of the TossEngine scene graph.
Author : Theo Morris
Mail : theo.morris@outlook.co.nz
***/

#pragma once

#include "Utils.h"
#include "Math.h"
#include "Serializable.h"
#include "Component.h"
#include "ISelectable.h"
#include "LayerManager.h"

namespace reactphysics3d
{
    class PhysicsWorld;
}

class MissingComponent;
class Collider;
struct Collision;
class Scene;
class LightManager;

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

    virtual void onUpdateInternal();

    GameObjectPtr getSharedOwner() const;

    // --- Inspector ---
    virtual void OnInspectorGUI() override;
    void OnDrawGizmos(UniformData data) const;
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

    template<typename C>
    C* getComponentExact() {
        auto it = m_components.find(std::type_index(typeid(C)));
        return (it != m_components.end()) ? static_cast<C*>(it->second) : nullptr;
    }
    template<typename C>
    const C* getComponentExact() const {
        auto it = m_components.find(std::type_index(typeid(C)));
        return (it != m_components.end()) ? static_cast<const C*>(it->second) : nullptr;
    }

    template<typename T>
    T* getComponent() {
        if (auto* p = getComponentExact<T>()) return p;
        for (auto& [_, comp] : m_components)
            if (auto* casted = dynamic_cast<T*>(comp)) return casted;
        return nullptr;
    }
    template<typename T>
    const T* getComponent() const {
        if (auto* p = getComponentExact<T>()) return p;
        for (auto& [_, comp] : m_components)
            if (auto* casted = dynamic_cast<const T*>(comp)) return casted;
        return nullptr;
    }

    // Variadic helpers must be const
    template <typename... Cs>
    bool hasComponent() const {
        static_assert(sizeof...(Cs) > 0, "hasComponent needs at least one type");
        return ((getComponent<Cs>() != nullptr) && ...);
    }

    template <typename... Cs>
    bool hasAnyComponent() const {
        static_assert(sizeof...(Cs) > 0, "hasAnyComponent needs at least one type");
        return ((getComponent<Cs>() != nullptr) || ...);
    }

    // Optional const bulk getter
    template <typename... Cs>
    std::tuple<const Cs*...> getComponents() const {
        return std::make_tuple(getComponent<Cs>()...);
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

    Scene* getScene() const;
    LightManager* getLightManager() const;
    void setScene(Scene* _scene);
    void OnTransformPositionChanged();
    void OnTransformRotationChanged();
    void OnTransformScaleChanged();

    bool Delete(bool deleteSelf = true) override;
    bool GetActive() const;

    // --- Physics ---

    virtual reactphysics3d::PhysicsWorld* getWorld();
    void CallOnCollisionEnterCallbacks(Collision& collision) const;
    void CallOnCollisionStayCallbacks(Collision& collision) const;
    void CallOnCollisionExitCallbacks(Collision& collision) const;
    void CallOnTriggerEnterCallbacks(Collider* other) const;
    void CallOnTriggerExitCallbacks(Collider* other) const;

protected:
    std::map<std::type_index, Component*> m_components;               //!< Active components
    std::map<std::string, MissingComponent*> m_missingComponents;     //!< Components not loaded (e.g., missing DLL)
    std::vector<Component*> componentsToDestroy;
    std::vector<MissingComponent*> missingComponetsToDestroy;

    bool hasStarted = false;
    bool m_finishedCreation = false;

    Scene* m_scene = nullptr;

    size_t m_id = 0;          //!< Unique identifier
    char tagBuffer[128];      //!< Internal buffer for tag editing

    //Editor stuff probably move idk was kinda required
    Vector3 editorEuler;      //!< Editor-facing rotation data
    Component* selectedComponent = nullptr;
};