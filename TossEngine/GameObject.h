#pragma once
#include "Utils.h"
#include "Math.h"
#include "Serializable.h"
#include "Component.h"
#include "GameObjectManager.h"
#include "ISelectable.h"

/**
 * @class GameObject
 * @brief Represents an object for OpenGl with its own update and onCreate functions.
 */
class TOSSENGINE_API GameObject : public Serializable, public ISelectable
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

    string name = "GameObject";
    Transform m_transform;

    // Serialize the GameObject to JSON
    virtual json serialize() const override;

    // Deserialize the GameObject from JSON
    virtual void deserialize(const json& data) override;

    virtual void OnInspectorGUI() override;

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
    LightManager* getLightManager();

    /**
     * @brief Sets the GameObjectManager that manages this GameObject.
     * @param GameObjectManager A pointer to the GameObjectManager.
     */
    void setGameObjectManager(GameObjectManager* gameObjectManager);

    /**
     * @brief Releases the GameObject, preparing it for destruction.
     */
    bool Delete(bool deleteSelf = true);

    /**
     * @brief Called when the GameObject is created.
     * Can be overridden by derived classes to perform initialization.
     */
    virtual void onCreate();

    /**
     * @brief Called when the game is started right before the first update frame.
     */
    virtual void onStart();

    /**
     * @brief Called right after start.
     */
    virtual void onLateStart();

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

    void onUpdateInternal();

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

    Component* addComponent(string componentType, const json& data = nullptr);
    void removeComponent(Component* component);

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
    
    template <typename Component>
    Component* getComponentInChildren()
    {
        // Check if this GameObject itself has the component.
        if (Component* comp = getComponent<Component>()) {
            return comp;
        }

        // Otherwise, loop over each child transform.
        for (Transform* childTransform : m_transform.children)
        {
            // Assuming each Transform has a pointer back to its owning GameObject.
            if (childTransform->gameObject)
            {
                // Recursively call getComponentInChildren on the child GameObject.
                if (Component* comp = childTransform->gameObject->getComponentInChildren<Component>())
                {
                    return comp;
                }
            }
        }

        return nullptr;
    }

    std::map<std::type_index, Component*>& getAllComponents() {
        return m_components;
    }

    bool tryDeleteSelectedComponent();

protected:
    vector<Component*> componentsToDestroy;
    std::map<std::type_index, Component*> m_components;

    GameObjectManager* m_gameObjectManager = nullptr; // Pointer to the GameObjectManager managing this GameObject.

    Component* selectedComponent = nullptr; 
private:
    size_t m_id = 0; // Unique identifier for the GameObject.
};