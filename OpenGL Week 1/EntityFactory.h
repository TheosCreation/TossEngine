#pragma once
#include "Utils.h"
#include "Entity.h"


class EntityFactory
{
public:
    using CreateEntityFunc = std::function<GameObject*()>; 
    EntityFactory() = default; // Default constructor, no need to initialize a map


    template <typename T>
    void registerEntity()
    {
        static_assert(std::is_base_of<GameObject, T>::value, "T must derive from Entity class");
        std::string typeName = getClassName(typeid(T));  // Ensure consistent type name usage

        std::cout << "Registering: " << typeName << std::endl; // Debug
        m_entityCreators.emplace(typeName, []() -> GameObject* { return new T(); });
    }

    GameObject* createEntity(const std::string& typeName)
    {
        std::cout << "Creating entity of type: " << typeName << std::endl;
        auto it = m_entityCreators.find(typeName);

        if (it == m_entityCreators.end()) {
            std::cout << "Entity type not found!\n";
            return nullptr;
        }

        return it->second(); // Call stored function
    }

private:
    std::map<std::string, CreateEntityFunc> m_entityCreators;
};