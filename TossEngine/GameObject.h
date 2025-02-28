#pragma once
#include "Utils.h"
#include "Math.h"
#include "Serializable.h"
#include "Component.h"

class GameObjectManager;

/**
 * @class GameObject
 * @brief Represents an object for OpenGl with its own update and onCreate functions.
 */
class TOSSENGINE_API GameObject : public Serializable
{
public:
    /**
     * @brief Constructor for the GameObject class.
     */
    GameObject();

    GameObject(const GameObject& other); //Copy Constructor

    /**
     * @brief Destructor for the GameObject class.
     */
    ~GameObject();

    Transform m_transform;

    // Serialize the GameObject to JSON
    virtual json serialize() const override;

    // Deserialize the GameObject from JSON
    virtual void deserialize(const json& data) override;

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
    virtual void onCreate();

    /**
     * @brief Called every frame to update the GameObject at a fixed frame rate.
     * Can be overridden by derived classes to implement custom behavior.
     * @param deltaTime The time elapsed since the last frame.
     */
    virtual void onFixedUpdate(float fixedDeltaTime);

    /**
     * @brief Called every frame to update the GameObject.
     * Can be overridden by derived classes to implement custom behavior.
     * @param deltaTime The time elapsed since the last frame.
     */
    virtual void onUpdate(float deltaTime);

    /**
     * @brief Called every frame after all Update functions have been called.
     * Can be overridden by derived classes to implement custom behavior.
     * @param deltaTime The time elapsed since the last frame.
     */
    virtual void onLateUpdate(float deltaTime);

    // Add components to the game object
    template <typename Component>
    Component* addComponent()
    {
        auto component = new Component();
        std::type_index typeIndex(typeid(Component)); 

        component->setOwner(this);
        component->onCreate();

        m_components.emplace(typeIndex, component);

        return static_cast<Component*>(m_components[typeIndex]);
    }

    template <typename Component>
    Component* getComponent()
    {
        std::type_index typeIndex(typeid(Component));
        auto it = m_components.find(typeIndex);

        if (it != m_components.end()) {
            return static_cast<Component*>(it->second);
        }
        return nullptr;
    }

    //Component* addCSharpComponent(const std::string& typeName);

protected:
    std::map<std::type_index, Component*> m_components;

    GameObjectManager* m_gameObjectManager = nullptr; // Pointer to the GameObjectManager managing this GameObject.

private:
    size_t m_id = 0; // Unique identifier for the GameObject.
};