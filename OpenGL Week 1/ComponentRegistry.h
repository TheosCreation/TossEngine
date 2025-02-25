#pragma once
#include <unordered_map>
#include <functional>
#include <memory>
#include <string>
#include "Component.h"

class ComponentRegistry
{
public:

    /**
     * @brief Provides access to the singleton instance of ComponentRegistry.
     * @return A reference to the ComponentRegistry instance.
     */
    static ComponentRegistry& GetInstance()
    {
        static ComponentRegistry instance;
        return instance;
    }

    // Delete the copy constructor and assignment operator to prevent copying
    ComponentRegistry(const ComponentRegistry& other) = delete;
    ComponentRegistry& operator=(const ComponentRegistry& other) = delete;

    /**
     * @brief Registers a component type with a creation function.
     * @tparam T The type of the component to register.
     */
    template <typename T>
    void registerComponent()
    {
        static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");
        std::string typeName = getClassName(typeid(T));  // Ensure consistent type name usage

        m_componentCreators[typeName] = []() -> std::unique_ptr<Component> {
            return std::make_unique<T>();
            };
    }

    /**
     * @brief Creates a component by its type name.
     * @param typeName The name of the component type to create.
     * @return A unique pointer to the created component, or nullptr if the type is not registered.
     */
    std::unique_ptr<Component> createComponent(const std::string& typeName)
    {
        auto it = m_componentCreators.find(typeName);
        if (it != m_componentCreators.end())
        {
            return it->second(); // Call the creation function
        }
        return nullptr; // Component type not found
    }

    /**
     * @brief Checks if a component type is registered.
     * @param typeName The name of the component type to check.
     * @return True if the component type is registered, false otherwise.
     */
    bool isComponentRegistered(const std::string& typeName) const
    {
        return m_componentCreators.find(typeName) != m_componentCreators.end();
    }

private:
    // Map of component type names to their creation functions
    std::unordered_map<std::string, std::function<std::unique_ptr<Component>()>> m_componentCreators;

    /**
     * @brief Constructor for the ComponentRegistry class.
     */
    ComponentRegistry() = default;

    /**
     * @brief Destructor for the ComponentRegistry class.
     */
    ~ComponentRegistry() = default;
};