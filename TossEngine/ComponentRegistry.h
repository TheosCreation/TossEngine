#pragma once
#include <unordered_map>
#include <functional>
#include <memory>
#include <string>
#include "Utils.h"

class Component;

class TOSSENGINE_API ComponentRegistry
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

        std::string typeName = getClassName(typeid(T));
        std::type_index currentType = std::type_index(typeid(T));

        auto it = m_componentTypes.find(typeName);

        if (it != m_componentTypes.end()) {
            if (it->second == currentType) {
                // Already registered, identical — skip
                return;
            }
            else {
                Debug::Log("Updating component type: " + typeName);
            }
        }
        else {
            Debug::Log("Registering new component type: " + typeName);
        }

        // (Re)register the component
        m_componentTypes[typeName] = currentType;
        m_componentCreators[typeName] = []() -> Component* {
            return new T();
            };
    }

    std::vector<std::string> getRegisteredComponentNames() const;

    /**
     * @brief Creates a component by its type name.
     * @param typeName The name of the component type to create.
     * @return A unique pointer to the created component, or nullptr if the type is not registered.
     */
    Component* createComponent(const std::string& typeName)
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

    void CleanUp();

private:
    // Map of component type names to their creation functions
    std::unordered_map<std::string, std::function<Component* ()>> m_componentCreators; 
    std::unordered_map<std::string, std::optional<std::type_index>> m_componentTypes;


    /**
     * @brief Constructor for the ComponentRegistry class.
     */
    ComponentRegistry() = default;

    /**
     * @brief Destructor for the ComponentRegistry class.
     */
    ~ComponentRegistry() = default;
};

#define REGISTER_COMPONENT(T) \
    static bool _registered_##T = []() { \
        ComponentRegistry::GetInstance().registerComponent<T>(); \
        return true; \
    }();
