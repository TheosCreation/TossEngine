#pragma once
#include "Utils.h"
#include "Math.h"
#include "Serializable.h"
#include "ComponentRegistry.h"
#include "Component.h"

class GameObjectManager;

/**
 * @class GameObject
 * @brief Represents an object for OpenGl with its own update and onCreate functions.
 */
class GameObject : public Serializable
{
public:
    /**
     * @brief Constructor for the GameObject class.
     */
    GameObject();

    /**
     * @brief Destructor for the GameObject class.
     */
    ~GameObject();

    Transform m_transform;

    // Serialize the GameObject to JSON
    virtual json serialize() const override
    {
        json componentsJson = json::array();
        for (const auto& pair : m_components)
        {
            componentsJson.push_back(pair.second->serialize());
        }

        return {
            {"type", getClassName(typeid(*this))}, // Use typeid to get the class name
            {"transform", {
                {"position", {m_transform.position.x, m_transform.position.y, m_transform.position.z}},
                {"rotation", {m_transform.rotation.x, m_transform.rotation.y, m_transform.rotation.z, m_transform.rotation.w}},
                {"scale", {m_transform.scale.x, m_transform.scale.y, m_transform.scale.z}}
            }},
            {"components", componentsJson}
        };
    }

    // Deserialize the GameObject from JSON
    virtual void deserialize(const json& data) override
    {
        if (data.contains("transform"))
        {
            auto transformData = data["transform"];
            if (transformData.contains("position"))
            {
                auto pos = transformData["position"];
                m_transform.position = Vector3(pos[0], pos[1], pos[2]);
            }
            if (transformData.contains("rotation"))
            {
                auto rot = transformData["rotation"];
                m_transform.rotation = Quaternion(rot[3], rot[0], rot[1], rot[2]);
            }
            if (transformData.contains("scale"))
            {
                auto scl = transformData["scale"];
                m_transform.scale = Vector3(scl[0], scl[1], scl[2]);
            }
        }

        if (data.contains("components"))
        {
            for (const auto& componentData : data["components"])
            {
                std::string componentType = componentData["type"];
                auto component = ComponentRegistry::GetInstance().createComponent(componentType);
                if (component)
                {
                    component->setOwner(this);
                    component->onCreate();
                    component->deserialize(componentData);
                    m_components[std::type_index(typeid(*component))] = std::move(component);
                }
            }
        }
    }

    /**
     * @brief Gets the unique identifier of the GameObject.
     * @return The unique identifier of the GameObject.
     */
    size_t getId();

    /**
     * @brief Sets the unique identifier of the GameObject.
     * @param id The unique identifier to set.
     */
    void setId(size_t id);

    /**
     * @brief Gets the GameObjectManager that manages this GameObject.
     * @return A pointer to the GameObjectManager.
     */
    GameObjectManager* getGameObjectManager();

    /**
     * @brief Sets the GameObjectManager that manages this GameObject.
     * @param GameObjectManager A pointer to the GameObjectManager.
     */
    void setGameObjectManager(GameObjectManager* gameObjectManager);

    /**
     * @brief Releases the GameObject, preparing it for destruction.
     */
    void release();

    /**
     * @brief Called when the GameObject is created.
     * Can be overridden by derived classes to perform initialization.
     */
    virtual void onCreate() {}

    /**
     * @brief Called every frame to update the GameObject at a fixed frame rate.
     * Can be overridden by derived classes to implement custom behavior.
     * @param deltaTime The time elapsed since the last frame.
     */
    virtual void onFixedUpdate(float fixedDeltaTime) 
    {
        for (auto& pair : m_components) {
            pair.second->onFixedUpdate(fixedDeltaTime);
        }
    }

    /**
     * @brief Called every frame to update the GameObject.
     * Can be overridden by derived classes to implement custom behavior.
     * @param deltaTime The time elapsed since the last frame.
     */
    virtual void onUpdate(float deltaTime)
    {
        for (auto& pair : m_components) {
            pair.second->onUpdate(deltaTime);
        }
    }

    /**
     * @brief Called every frame after all Update functions have been called.
     * Can be overridden by derived classes to implement custom behavior.
     * @param deltaTime The time elapsed since the last frame.
     */
    virtual void onLateUpdate(float deltaTime) {}

    // Add components to the game object
    template <typename Component>
    Component* addComponent() {
        // Create a new unique_ptr for the component
        auto component = std::make_unique<Component>();

        std::type_index typeIndex(typeid(Component)); // Store using compile-time type
        component->setOwner(this);
        component->onCreate();

        // Move ownership into the component map
        m_components[typeIndex] = std::move(component);

        // Return the raw pointer to the stored component
        return static_cast<Component*>(m_components[typeIndex].get());
    }

    template <typename Component>
    Component* getComponent() {
        std::type_index typeIndex(typeid(Component));
        auto it = m_components.find(typeIndex);

        if (it != m_components.end()) {
            return static_cast<Component*>(it->second.get());
        }
        return nullptr;
    }

protected:
    std::map<std::type_index, ComponentPtr> m_components;

    GameObjectManager* m_gameObjectManager = nullptr; // Pointer to the GameObjectManager managing this GameObject.

private:
    size_t m_id = 0; // Unique identifier for the GameObject.
};